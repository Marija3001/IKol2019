// Kol1View.h : interface of the CKol1View class  (I kolokvijum 2019 - Transformer)
#pragma once
#include "DImage.h"

class CKol1View : public CView
{
protected:
	CKol1View() noexcept;
	DECLARE_DYNCREATE(CKol1View)

public:
	CKol1Doc* GetDocument() const;

	virtual void OnDraw(CDC* pDC);
	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);
	virtual ~CKol1View();

protected:
	// 1. slike
	DImage* m_pBody1;
	DImage* m_pArm1;
	DImage* m_pArm2;
	DImage* m_pLeg1;
	DImage* m_pLeg2;
	DImage* m_pBackground;

	// uglovi (u stepenima) - atributi View klase
	double m_angleArm1;   // Q / A
	double m_angleLeg1;   // Q / A
	double m_angleArm2;   // T / G
	double m_angleLeg2;   // R / F
	double m_angleBody;   // W / S

	// 2.
	void DrawScene(CDC* pDC);

	// 3., 4.
	void DrawBackground(CDC* pDC, CRect rc);
	void DrawImgTransparent(CDC* pDC, DImage* pImage);

	// 5.
	void Translate(CDC* pDC, float dX, float dY, bool rightMultiply);
	void Rotate(CDC* pDC, float angle, bool rightMultiply);   // angle u stepenima

	// 6. - 11.
	void DrawArm1(CDC* pDC);
	void DrawArm2(CDC* pDC);
	void DrawLeg1(CDC* pDC);
	void DrawLeg2(CDC* pDC);
	void DrawBody1(CDC* pDC);
	void DrawTransformer(CDC* pDC);

	// 12.
	void StepAngle(double& angle, double target, int direction);

protected:
	DECLARE_MESSAGE_MAP()
public:
	afx_msg BOOL OnEraseBkgnd(CDC* pDC);
	afx_msg void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags);
};

#ifndef _DEBUG
inline CKol1Doc* CKol1View::GetDocument() const
   { return reinterpret_cast<CKol1Doc*>(m_pDocument); }
#endif
