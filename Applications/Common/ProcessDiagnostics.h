#pragma once

// Current-process diagnostics for Windows / MSVC, C++17 or newer.
// Include after pch.h in an MFC project. Uses current Windows SDK headers.
// No OpenGL or DirectX device is required. GPU counters require WDDM support.
//
// Lifetime: Initialize(), Sample() about once per second, Shutdown().
// The destructor also calls Shutdown(). Noncopyable; use from ONE thread.
// Keep the object alive between samples: CPU/GPU utilization needs history.
// Sample() performs synchronous PDH collection. If collection causes UI
// stalls on your system, run the object on a worker and post snapshots to UI.
// Never call it for every render frame or mouse event.
//
// CPU: elapsed process kernel+user time / wall time / machine logical CPUs.
// This is a 0..100% machine-capacity scale, not percent of one core and not
// frequency-weighted CPU utility. Task Manager need not match it exactly.
// RAM: WorkingSet = resident private+shared pages; PrivateCommit = committed
// private memory, including pages that are not currently resident.
// PrivateWorkingSet is separately available on recent Windows versions.
// GPU: maximum valid engine utilization for THIS PID, across adapters.
// GPU memory: sum of THIS PID's dedicated/shared usage across adapters.
// GPU shared memory means system RAM used by GPU, not additional VRAM.
// These memory categories overlap; do not add them into a "total memory".
// Missing instances, invalid samples, or unsupported counters remain invalid
// (formatted as N/A), rather than being presented as measured zero usage.
//
// GPU setup uses English PDH names, independent of the Windows UI language.
// Wildcard arrays discover new GPU instances without rebuilding the query.
// Array storage and the status string are reused; no transient STL containers.
// PDH itself may allocate internally. The buffer grows only as needed.
//
// References:
// https://learn.microsoft.com/en-us/windows/win32/api/processthreadsapi/nf-processthreadsapi-getprocesstimes
// https://learn.microsoft.com/en-us/windows/win32/api/psapi/ns-psapi-process_memory_counters_ex2
// https://learn.microsoft.com/en-us/windows/win32/api/pdh/nf-pdh-pdhgetformattedcounterarrayw
// https://devblogs.microsoft.com/directx/gpus-in-the-task-manager/

#include <windows.h>
#include <psapi.h>
#include <pdh.h>
#include <pdhmsg.h>
#include <cstdint>
#include <cstdio>
#include <cstdlib>
#include <cwchar>
#include <cmath>

#pragma comment(lib, "pdh.lib")
#pragma comment(lib, "psapi.lib")

class ProcessDiagnostics
{

public:

	struct Snapshot
    {
        double CpuPercent = 0;
        double GpuPercent = 0;
        uint64_t WorkingSetBytes = 0;
        uint64_t PrivateCommitBytes = 0;
        uint64_t PrivateWorkingSetBytes = 0;
        uint64_t GpuDedicatedBytes = 0;
        uint64_t GpuSharedBytes = 0;
        DWORD GdiObjects = 0;
        DWORD UserObjects = 0;

        bool CpuValid = false;
        bool MemoryValid = false;
        bool PrivateWorkingSetValid = false;
        bool GpuValid = false;
        bool GpuDedicatedValid = false;
        bool GpuSharedValid = false;
        bool GdiValid = false;
        bool UserValid = false;
    };

    enum GpuCounter { Engine, DedicatedMemory, SharedMemory, CounterCount };

    ProcessDiagnostics() = default;
    ~ProcessDiagnostics() { Shutdown(); }
    ProcessDiagnostics(const ProcessDiagnostics &) = delete;
    ProcessDiagnostics &operator=(const ProcessDiagnostics &) = delete;

    // False means CPU timing could not initialize. Memory can still work.
    // GPU failure is independent: inspect validity bits / GetGpuStatus().
    bool Initialize()
    {
        Shutdown();
        m_Initialized = true;
        m_ProcessorCount = GetActiveProcessorCount(ALL_PROCESSOR_GROUPS);
        if (!m_ProcessorCount)
        {
            SYSTEM_INFO info = {};
            GetSystemInfo(&info);
            m_ProcessorCount = info.dwNumberOfProcessors;
        }
        if (!m_ProcessorCount)
            m_ProcessorCount = 1;

        swprintf_s(m_PidPrefix, L"pid_%lu_", GetCurrentProcessId());
        m_PidPrefixLength = wcslen(m_PidPrefix);
        bool timing = QueryPerformanceFrequency(&m_Frequency) != FALSE;
        if (timing && m_Frequency.QuadPart > 0)
            ReadCpu(); // Establish baseline; no percentage yet.

        PDH_STATUS status = PdhOpenQueryW(nullptr, 0, &m_Query);
        if (status == ERROR_SUCCESS)
        {
            static const wchar_t *paths[CounterCount] =
            {
                L"\\GPU Engine(*)\\Utilization Percentage",
                L"\\GPU Process Memory(*)\\Dedicated Usage",
                L"\\GPU Process Memory(*)\\Shared Usage"
            };
            bool any = false;
            for (int i = 0; i < CounterCount; i++)
            {
                m_GpuStatus[i] = PdhAddEnglishCounterW(
                    m_Query, paths[i], 0, &m_Counters[i]);
                if (m_GpuStatus[i] == ERROR_SUCCESS)
                    any = true;
                else
                    m_Counters[i] = nullptr;
            }
            if (any)
                PdhCollectQueryData(m_Query); // Prime rate counters.
            else
            {
                PdhCloseQuery(m_Query);
                m_Query = nullptr;
            }
        }
        else
        {
            m_Query = nullptr;
            for (int i = 0; i < CounterCount; i++)
                m_GpuStatus[i] = status;
        }
        return m_HaveCpuBaseline;
    }

    void Shutdown()
    {
        if (m_Query)
            PdhCloseQuery(m_Query); // Also closes its counters.

		m_Query = nullptr;

		for (int i = 0; i < CounterCount; i++)
        {
            m_Counters[i] = nullptr;
            m_GpuStatus[i] = PDH_NO_DATA;
        }

		free(m_Buffer);
        m_Buffer = nullptr;
        m_BufferSize = 0;
        m_Frequency = {};
        m_PreviousWall = {};
        m_PreviousCpu = 0;
        m_HaveCpuBaseline = false;
        m_Initialized = false;
        m_Data = {};
        m_Text[0] = '\0';
    }

    const Snapshot &Sample()
    {
        m_Data = {}; // Never return stale measurements as current.
        if (!m_Initialized)
            return m_Data;

        ReadCpu();
        ReadMemory();

        SetLastError(ERROR_SUCCESS);
        m_Data.GdiObjects = GetGuiResources(GetCurrentProcess(), GR_GDIOBJECTS);
        m_Data.GdiValid = m_Data.GdiObjects != 0 || GetLastError() == ERROR_SUCCESS;

		SetLastError(ERROR_SUCCESS);
        m_Data.UserObjects = GetGuiResources(GetCurrentProcess(), GR_USEROBJECTS);
        m_Data.UserValid = m_Data.UserObjects != 0 || GetLastError() == ERROR_SUCCESS;

        if (m_Query)
        {
            PDH_STATUS status = PdhCollectQueryData(m_Query);

			if (status == ERROR_SUCCESS)
            {
                ReadGpu(Engine);
                ReadGpu(DedicatedMemory);
                ReadGpu(SharedMemory);
            }
            else
            {
                for (int i = 0; i < CounterCount; i++)
                    if (m_Counters[i])
                        m_GpuStatus[i] = status;
            }
        }

		return m_Data;
    }

    const Snapshot &GetSnapshot() const { return m_Data; }

    // Last setup/read status; PDH_NO_DATA also represents no matching valid
    // PID sample. Call Initialize again to retry counters absent at setup.
    PDH_STATUS GetGpuStatus(GpuCounter counter) const
    {
        return (counter >= Engine) && (counter < CounterCount) ? m_GpuStatus[counter] : PDH_INVALID_ARGUMENT;
    }

    // Pointer remains valid until next FormatStatus()/Shutdown()/destruction.
    // ASCII output converts directly to CString in a normal MFC project.
    const TCHAR *FormatStatus()
    {
        TCHAR cpu[32] = _T("N/A"), gpu[32] = _T("N/A");
        TCHAR ws[32] = _T("N/A"), commit[32] = _T("N/A");
        TCHAR dedicated[32] = _T("N/A"), shared[32] = _T("N/A");

		constexpr double MiB = 1024.0 * 1024.0;

		if (m_Data.CpuValid)
            _sntprintf_s(cpu, sizeof(cpu), _T("%.1f%%"), m_Data.CpuPercent);

		if (m_Data.GpuValid)
            _sntprintf_s(gpu, sizeof(gpu), _T("%.1f%%"), m_Data.GpuPercent);

		if (m_Data.MemoryValid)
        {
            _sntprintf_s(ws, sizeof(ws), _T("%.0f"), m_Data.WorkingSetBytes / MiB);
            _sntprintf_s(commit, sizeof(commit), _T("%.0f"), m_Data.PrivateCommitBytes / MiB);
        }

		if (m_Data.GpuDedicatedValid)
            _sntprintf_s(dedicated, sizeof(dedicated), _T("%.0f"), m_Data.GpuDedicatedBytes / MiB);

		if (m_Data.GpuSharedValid)
            _sntprintf_s(shared, sizeof(shared), _T("%.0f"), m_Data.GpuSharedBytes / MiB);

		_sntprintf_s(m_Text, sizeof(m_Text),
            _T("CPU %s | GPU %s | WS %s MiB | Commit %s MiB | GPU D/S %s/%s MiB"),
            cpu, gpu, ws, commit, dedicated, shared);

		return m_Text;
    }

private:
    static uint64_t FileTimeValue(const FILETIME &time)
    {
        return (uint64_t(time.dwHighDateTime) << 32) | time.dwLowDateTime;
    }

    void ReadCpu()
    {
        FILETIME created, exited, kernel, user;
        LARGE_INTEGER now;
        if (m_Frequency.QuadPart <= 0 ||
            !GetProcessTimes(GetCurrentProcess(), &created, &exited, &kernel, &user) ||
            !QueryPerformanceCounter(&now))
        {
            m_HaveCpuBaseline = false;
            return;
        }
        uint64_t cpu = FileTimeValue(kernel) + FileTimeValue(user);
        if (m_HaveCpuBaseline && cpu >= m_PreviousCpu &&
            now.QuadPart > m_PreviousWall.QuadPart)
        {
            double seconds = double(now.QuadPart - m_PreviousWall.QuadPart) /
                double(m_Frequency.QuadPart);
            double percent = 100.0 * (double(cpu - m_PreviousCpu) / 10000000.0) /
                (seconds * m_ProcessorCount);
            m_Data.CpuPercent = percent > 100.0 ? 100.0 : percent;
            m_Data.CpuValid = true;
        }
        m_PreviousWall = now;
        m_PreviousCpu = cpu;
        m_HaveCpuBaseline = true;
    }

    void ReadMemory()
    {
        // EX2 requires recent SDK headers and updated Windows 10/11.
        // Runtime fallback preserves ordinary WS/commit on older Windows.
        PROCESS_MEMORY_COUNTERS_EX2 ex2 = {};
        ex2.cb = sizeof(ex2);
        if (GetProcessMemoryInfo(GetCurrentProcess(),
            reinterpret_cast<PROCESS_MEMORY_COUNTERS *>(&ex2), sizeof(ex2)))
        {
            m_Data.WorkingSetBytes = ex2.WorkingSetSize;
            m_Data.PrivateCommitBytes = ex2.PrivateUsage;
            m_Data.PrivateWorkingSetBytes = ex2.PrivateWorkingSetSize;
            m_Data.MemoryValid = true;
            m_Data.PrivateWorkingSetValid = true;
        }
        else
        {
            PROCESS_MEMORY_COUNTERS_EX ex = {};
            ex.cb = sizeof(ex);
            if (GetProcessMemoryInfo(GetCurrentProcess(),
                reinterpret_cast<PROCESS_MEMORY_COUNTERS *>(&ex), sizeof(ex)))
            {
                m_Data.WorkingSetBytes = ex.WorkingSetSize;
                m_Data.PrivateCommitBytes = ex.PrivateUsage;
                m_Data.MemoryValid = true;
            }
        }
    }

    bool ReadArray(GpuCounter counter, DWORD format, DWORD &count)
    {
        count = 0;
        if (!m_Counters[counter])
            return false;
        // Retry sizing if the instance list changes. Do not trust the size
        // returned for an undersized nonzero buffer: PDH documents that case.
        for (int attempt = 0; attempt < 3; attempt++)
        {
            DWORD needed = 0;
            PDH_STATUS status = PdhGetFormattedCounterArrayW(
                m_Counters[counter], format, &needed, &count, nullptr);
            m_GpuStatus[counter] = status;
            if (status != PDH_MORE_DATA)
                return status == ERROR_SUCCESS && count == 0;
            if (needed > m_BufferSize)
            {
                void *buffer = realloc(m_Buffer, needed);
                if (!buffer)
                {
                    m_GpuStatus[counter] = PDH_MEMORY_ALLOCATION_FAILURE;
                    return false;
                }
                m_Buffer = buffer;
                m_BufferSize = needed;
            }
            DWORD size = m_BufferSize;
            status = PdhGetFormattedCounterArrayW(m_Counters[counter], format,
                &size, &count, static_cast<PDH_FMT_COUNTERVALUE_ITEM_W *>(m_Buffer));
            m_GpuStatus[counter] = status;
            if (status == ERROR_SUCCESS)
                return true;
            if (status != PDH_MORE_DATA && status != PDH_INVALID_ARGUMENT)
                return false;
        }
        return false;
    }

    void ReadGpu(GpuCounter counter)
    {
        DWORD count = 0;
        DWORD format = (counter == Engine ? PDH_FMT_DOUBLE : PDH_FMT_LARGE) |
            PDH_FMT_NOCAP100 | PDH_FMT_NOSCALE;
        if (!ReadArray(counter, format, count))
            return;
        const auto *items = static_cast<const PDH_FMT_COUNTERVALUE_ITEM_W *>(m_Buffer);
        bool found = false, invalid = false;
        double peak = 0;
        uint64_t bytes = 0;
        for (DWORD i = 0; i < count; i++)
        {
            // Trailing underscore prevents pid_12 matching pid_123.
            if (!items[i].szName ||
                _wcsnicmp(items[i].szName, m_PidPrefix, m_PidPrefixLength) != 0)
                continue;
            const PDH_FMT_COUNTERVALUE &value = items[i].FmtValue;
            if (value.CStatus != PDH_CSTATUS_VALID_DATA &&
                value.CStatus != PDH_CSTATUS_NEW_DATA)
            {
                invalid = true;
                continue;
            }
            if (counter == Engine)
            {
                if (!std::isfinite(value.doubleValue) || value.doubleValue < 0)
                {
                    invalid = true;
                    continue;
                }
                if (value.doubleValue > peak)
                    peak = value.doubleValue;
            }
            else
            {
                if (value.largeValue < 0)
                {
                    invalid = true;
                    continue;
                }
                bytes += uint64_t(value.largeValue);
            }
            found = true;
        }
        // Do not quietly underreport a partial aggregate during churn/warmup.
        if (!found || invalid)
        {
            m_GpuStatus[counter] = PDH_NO_DATA;
            return;
        }
        m_GpuStatus[counter] = ERROR_SUCCESS;
        if (counter == Engine)
        {
            m_Data.GpuPercent = peak > 100.0 ? 100.0 : peak;
            m_Data.GpuValid = true;
        }
        else if (counter == DedicatedMemory)
        {
            m_Data.GpuDedicatedBytes = bytes;
            m_Data.GpuDedicatedValid = true;
        }
        else
        {
            m_Data.GpuSharedBytes = bytes;
            m_Data.GpuSharedValid = true;
        }
    }

    bool m_Initialized = false;
    bool m_HaveCpuBaseline = false;
    DWORD m_ProcessorCount = 1;
    LARGE_INTEGER m_Frequency = {};
    LARGE_INTEGER m_PreviousWall = {};
    uint64_t m_PreviousCpu = 0;
    PDH_HQUERY m_Query = nullptr;
    PDH_HCOUNTER m_Counters[CounterCount] = {};
    PDH_STATUS m_GpuStatus[CounterCount] = { (PDH_STATUS)PDH_NO_DATA, (PDH_STATUS)PDH_NO_DATA, (PDH_STATUS)PDH_NO_DATA };
    void *m_Buffer = nullptr;
    DWORD m_BufferSize = 0;
    wchar_t m_PidPrefix[32] = {};
    size_t m_PidPrefixLength = 0;
    Snapshot m_Data;
    TCHAR m_Text[384] = {};
};
