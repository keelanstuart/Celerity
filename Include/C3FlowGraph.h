// **************************************************************
// Celerity v3 Game / Visualization Engine Source File
//
// Copyright © 2001-2023, Keelan Stuart


#pragma once

#include <GenIO.h>

namespace c3
{

	class FlowNode;

	class FlowGraph
	{

	public:

		typedef enum eExecResult
		{
			SUCCESS = 0,
			NO_STARTER,

			NUMRESULTS
		} EExecResult;

		/// Frees any resources that the node may have allocated
		virtual void Release() = NULL;

		virtual size_t GetNumNodes() const = NULL;

		virtual FlowNode *GetNode(size_t index) const = NULL;

		virtual size_t FindNode(const FlowNode *pfn) const = NULL;

		virtual FlowNode *GetNodeByGuid(GUID g) const = NULL;

		virtual void AddNode(FlowNode *pfn) = NULL;

		virtual void RemoveNode(FlowNode *pfn) = NULL;

		virtual EExecResult Execute() = NULL;

		/// Loads the FlowGraph from a stream
		virtual bool Load(genio::IInputStream *is) = NULL;

		/// Saves an FlowGraph to a stream
		virtual bool Save(genio::IOutputStream *os, props::TFlags64 saveflags) const = NULL;

		/// Called once Load has finished
		virtual void PostLoad() = NULL;

	};
};