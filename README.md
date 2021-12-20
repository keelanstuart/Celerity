# Celerity
Celerity - a game / visualization engine for Windows, written in C++, currently in it's third-ish iteration

Once you have cloned the repo, run SetupCelerityEnv.bat to create / update the C3_SDK environment variable.

Next, load the main Celerity solution in Visual Studio and build the "Bootstrap (Debug)" target. Be prepared: it will be building the OpenGL wrapper code, which involves downloading and parsing header files and documentation (yes, the resulting code includes comments detailing the version of OpenGL the function comes from and what it does). Even if you leave it in the Bootstrap configuration, a CRC is generated based on the .h files and the parameters used to create it which, together, prevent re-processing of all the OpenGL headers. Even so, the initial download is slow enough that you'll want to switch to a Debug / Release config.

![image](https://github.com/keelanstuart/Celerity/blob/master/screen_20211219.png)
Sponza with 300 deferred lights

______________________________________________

Some background...

Celerity has been around since roughly 2001, beginning life as a DirectX 8.1 engine (which got ported to the OG XBox)... eventually moving to DirectX 9... and finally being ported to OpenGL. The initial reasons for using DirectX (better drivers, rapid adoption of new hardware features, and lots of helper library functions in D3DX) went away sometime in the last decade, or so, and have finally prompted a re-write using OpenGL. Why not Vulkan or DX12? I think they (the hardware vendors and Microsoft) have tried to edge out smaller-scale studios by shifting the burden of software development away from themselves... and I'm not saying they're unwise to do this, but because of that I believe OpenGL, at this point, provides a better platform for "lone wolves."

No one should expect Celerity to become the next big thing, but it was licensed to two companies in the past (in the DX8.1 form), and so therefore may have the pedigree to be a platform for small-scale PC-based graphics applications.

Stay tuned for updates.

To see some of the things Celerity has been used to create, check out this promotional video from my old company, Inventoractive: https://www.youtube.com/watch?v=mlXiiIEqRsw
