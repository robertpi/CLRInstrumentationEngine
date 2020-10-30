// Copyright (c) Microsoft Corporation. All rights reserved.
// Licensed under the MIT License.

#pragma once


namespace MicrosoftInstrumentationEngine
{
    class CInstruction;
    class CBranchTargetInfo
    {
        
    public:
        HRESULT SetBranchTarget(_In_ CInstruction* pBranch, _In_ CInstruction* pTarget);
        HRESULT RetargetBranches(_In_ CInstruction* pOriginalInstruction, _In_ CInstruction* pNewInstruction);
        void Disconnect();
        bool IsConnected();

        virtual ~CBranchTargetInfo() = default;
    private:
        using TargetList = std::unordered_set<CInstruction*>;
        std::unordered_map<CInstruction*, TargetList> m_targetMap;
    };
}