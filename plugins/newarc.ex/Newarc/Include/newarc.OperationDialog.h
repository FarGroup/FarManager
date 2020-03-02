#include "newarc.h"

class OperationDialog {

private:	
	
	string m_strPanelTitle;

	double m_dPercent;
	double m_dTotalPercent;

	string m_strTitle;

	string m_strSrcFileName;
	string m_strDestFileName;

	int m_nOperation;
	int m_nStage;

	bool m_bShowSingleFileProgress;

public:

	OperationDialog();

	void SetShowSingleFileProgress(bool bShow);
	void SetOperation(int nOperation, int nStage);
	void SetPercent(double dPercent, double dTotalPercent);
	void SetTitle(string strTitle);
	void SetSrcFileName(string strFileName);
	void SetDestFileName(string strFileName);
	void Show();
};
