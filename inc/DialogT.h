#pragma once

template <UINT _ResID=0, bool _OnOK = false, bool _OnCancel = true>
class CDialogT : public CDialog
{
public:
	CDialogT()
		: CDialog(_ResID)
	{
	}

private:
	void OnOK() override
	{
		if (_OnOK)
		{
			__super::OnOK();
		}
	}

	void OnCancel() override
	{
		if (_OnCancel)
		{
			__super::OnCancel();
		}
	}
};
