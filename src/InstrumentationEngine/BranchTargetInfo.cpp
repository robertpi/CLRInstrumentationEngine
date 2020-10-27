#include "stdafx.h"
#include "refcount.h"
#include "Instruction.h"
#include "BranchTargetInfo.h"

namespace MicrosoftInstrumentationEngine
{
    CBranchTargetInfo::CBranchTargetInfo(_In_ CInstruction* pInstruction) : m_pInstruction(pInstruction)
    {

    }

    HRESULT CBranchTargetInfo::GetInstance(CInstruction* pInstruction, CBranchTargetInfo** ppResult)
    {
        IfNullRet(pInstruction);
        IfNullRet(ppResult);

        const GUID* uuid = &__uuidof(CBranchTargetInfo);
        CComPtr<IUnknown> pUnknown;
        HRESULT hr;
        // Don't assert, it is common for there to not be target info.
        IfFailRetNoLog(pInstruction->GetDataItem(uuid, uuid, &pUnknown));
        CComQIPtr<CBranchTargetInfo> pTargetInfo = pUnknown;

        IfFalseRet(pTargetInfo != nullptr, E_UNEXPECTED);
        *ppResult = pTargetInfo.Detach();

        return S_OK;
    }

    HRESULT CBranchTargetInfo::GetOrCreateInstance(CInstruction* pInstruction, CBranchTargetInfo** ppResult)
    {
        IfNullRet(pInstruction);
        IfNullRet(ppResult);
        if (!SUCCEEDED(GetInstance(pInstruction, ppResult)))
        {
            CComPtr<CBranchTargetInfo> pResult;
            pResult.Attach(new CBranchTargetInfo(pInstruction));
            const GUID* uuid = &__uuidof(CBranchTargetInfo);
            pInstruction->SetDataItem(uuid, uuid, pResult);
            *ppResult = pResult.Detach();
        }

        return S_OK;
    }

    HRESULT CBranchTargetInfo::SetBranchTarget(_In_ CInstruction* pBranch, _In_ CInstruction* pTarget)
    {
        IfNullRet(pBranch);
        IfNullRet(pTarget);

        HRESULT hr;
        CComPtr<CBranchTargetInfo> pInfo;
        IfFailRet(CBranchTargetInfo::GetOrCreateInstance(pTarget, &pInfo));
        pInfo.p->m_branches.emplace(pBranch);
        return S_OK;
    }

    HRESULT CBranchTargetInfo::RetargetBranches(CInstruction* pOriginalInstruction, CInstruction* pNewInstruction)
    {
        IfNullRet(pOriginalInstruction);
        IfNullRet(pNewInstruction);

        CComPtr<CBranchTargetInfo> pTargetInfo;
        if (SUCCEEDED(CBranchTargetInfo::GetInstance(pOriginalInstruction, &pTargetInfo)))
        {
            return pTargetInfo->Retarget(pNewInstruction);
        }
        return S_OK;
    }

    HRESULT CBranchTargetInfo::Retarget(CInstruction* pNewInstruction)
    {
        HRESULT hr = S_OK;
        IfFailRet(Disconnect());

        for (const CComPtr<CInstruction>&branch : m_branches)
        {
            if (branch->GetIsBranchInternal())
            {
                CBranchInstruction* pBranch = (CBranchInstruction*)branch.p;

                CInstruction* pBranchTarget = pBranch->GetBranchTargetInternal();

                // If the branch target is the original instruction AND the branch is not the new instruction,
                // update it. Note the second condition is important because for things like leave instructions,
                // often the next instruction is the branch target and resetting this would create an infinite loop.
                //pBranch->ReplaceTarget(pNewInstruction);

                if (pBranchTarget == pNewInstruction)
                {
                    // reset it back to the original instruction.
                    IfFailRet(pBranch->SetBranchTarget(m_pInstruction));
                }
                else
                {
                    IfFailRet(pBranch->SetBranchTarget(pNewInstruction));
                }
            }
            else if (branch->GetIsSwitchInternal())
            {
                CComPtr<ISwitchInstruction> pSwitch;
                IfFailRet(branch->QueryInterface(__uuidof(ISwitchInstruction), (LPVOID*)&pSwitch));
                IfFailRet(pSwitch->ReplaceBranchTarget(m_pInstruction, pNewInstruction));
            }
        }

        return S_OK;
    }

    HRESULT CBranchTargetInfo::Disconnect()
    {
        const GUID* uuid = &__uuidof(CBranchTargetInfo);

        // break cycles.
        return m_pInstruction->SetDataItem(uuid, uuid, nullptr);
    }
}

