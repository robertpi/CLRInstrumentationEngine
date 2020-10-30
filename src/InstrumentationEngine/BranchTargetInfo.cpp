#include "stdafx.h"
#include "refcount.h"
#include "BranchTargetInfo.h"
#include "Instruction.h"

namespace MicrosoftInstrumentationEngine
{
    HRESULT CBranchTargetInfo::SetBranchTarget(_In_ CInstruction* pBranch, _In_ CInstruction* pTarget)
    {
        IfNullRet(pBranch);
        IfNullRet(pTarget);

        if (m_targetMap.find(pTarget) == m_targetMap.end())
        {
            m_targetMap[pTarget] = {};
        }

        m_targetMap[pTarget].emplace(pBranch);

        return S_OK;
    }

    HRESULT CBranchTargetInfo::RetargetBranches(_In_ CInstruction* pOriginalInstruction, _In_ CInstruction* pNewInstruction)
    {
        IfNullRet(pOriginalInstruction);
        IfNullRet(pNewInstruction);

        if (m_targetMap.find(pOriginalInstruction) == m_targetMap.end())
        {
            return S_OK;
        }

        HRESULT hr;

        for (CInstruction* branch : m_targetMap[pOriginalInstruction])
        {
            if (branch->GetIsBranchInternal())
            {
                CBranchInstruction* pBranch = (CBranchInstruction*)branch;

                CInstruction* pBranchTarget = pBranch->GetBranchTargetInternal();

                IfFailRet(pBranch->SetBranchTarget(pNewInstruction));
            }
            else if (branch->GetIsSwitchInternal())
            {
                CComPtr<ISwitchInstruction> pSwitch;
                IfFailRet(branch->QueryInterface(__uuidof(ISwitchInstruction), (LPVOID*)&pSwitch));
                IfFailRet(pSwitch->ReplaceBranchTarget(pOriginalInstruction, pNewInstruction));
            }
        }

        m_targetMap.erase(pOriginalInstruction);

        return S_OK;
    }


    void CBranchTargetInfo::Disconnect()
    {
        m_targetMap.clear();
    }
    
    bool CBranchTargetInfo::IsConnected()
    {
        return m_targetMap.size() > 0;
    }
}

