// Kol1View.cpp : implementation of the CKol1View class  (I kolokvijum 2019 - Transformer)
#include "pch.h"
#include "framework.h"
#ifndef SHARED_HANDLERS
#include "Kol1.h"
#endif

#include "Kol1Doc.h"
#include "Kol1View.h"
#define _USE_MATH_DEFINES
#include <math.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif

#define STEP_ANGLE 5.0      // korak za pojedinacne tastere (stepeni)
#define TRANSFORM_STEPS 20  // broj pritisaka strelice od automobila do robota

// Tacka na body1.png gde se kaci arm1 (gornji desni zglob). Nije data u
// zadatku - procenjeno sa slike, po potrebi korigovati.
#define BODY_ARM_X 214.0f
#define BODY_ARM_Y 85.0f

// Polozaj korena (zglob leg2 (35,60)) u prozoru
#define ROOT_X 250.0f
#define ROOT_FROM_BOTTOM 200.0f

// Uglovi u polozaju garda (robot sa pocetka blanketa); automobil = sve 0
#define ROBOT_LEG2  110.0
#define ROBOT_LEG1 -220.0
#define ROBOT_BODY   20.0
#define ROBOT_ARM1   90.0
#define ROBOT_ARM2  -90.0

IMPLEMENT_DYNCREATE(CKol1View, CView)

BEGIN_MESSAGE_MAP(CKol1View, CView)
	ON_WM_ERASEBKGND()
	ON_WM_KEYDOWN()
END_MESSAGE_MAP()

// ---------------------------------------------------------------
// 1. Ucitavanje slika u konstruktoru, dealokacija u destruktoru
// ---------------------------------------------------------------
CKol1View::CKol1View() noexcept
{
	m_pBody1 = new DImage();      m_pBody1->Load(CString("body1.png"));
	m_pArm1 = new DImage();       m_pArm1->Load(CString("arm1.png"));
	m_pArm2 = new DImage();       m_pArm2->Load(CString("arm2.png"));
	m_pLeg1 = new DImage();       m_pLeg1->Load(CString("leg1.png"));
	m_pLeg2 = new DImage();       m_pLeg2->Load(CString("leg2.png"));
	m_pBackground = new DImage(); m_pBackground->Load(CString("background.jpg"));

	// pocetno stanje: automobil
	m_angleArm1 = 0.0;
	m_angleLeg1 = 0.0;
	m_angleArm2 = 0.0;
	m_angleLeg2 = 0.0;
	m_angleBody = 0.0;
}

CKol1View::~CKol1View()
{
	delete m_pBody1;
	delete m_pArm1;
	delete m_pArm2;
	delete m_pLeg1;
	delete m_pLeg2;
	delete m_pBackground;
}

BOOL CKol1View::PreCreateWindow(CREATESTRUCT& cs)
{
	return CView::PreCreateWindow(cs);
}

// ---------------------------------------------------------------
// 5. Transformacije
//    rightMultiply = false -> MWT_LEFTMULTIPLY  (lokalna, u odnosu na tekuci k.s.)
//    rightMultiply = true  -> MWT_RIGHTMULTIPLY (globalna)
// ---------------------------------------------------------------
void CKol1View::Translate(CDC* pDC, float dX, float dY, bool rightMultiply)
{
	XFORM form = { 1.0f, 0.0f, 0.0f, 1.0f, dX, dY };
	pDC->ModifyWorldTransform(&form, rightMultiply ? MWT_RIGHTMULTIPLY : MWT_LEFTMULTIPLY);
}

void CKol1View::Rotate(CDC* pDC, float angle, bool rightMultiply)
{
	float rad = (float)(angle * M_PI / 180.0);
	XFORM form = { cosf(rad), sinf(rad), -sinf(rad), cosf(rad), 0.0f, 0.0f };
	pDC->ModifyWorldTransform(&form, rightMultiply ? MWT_RIGHTMULTIPLY : MWT_LEFTMULTIPLY);
}

// ---------------------------------------------------------------
// 3. Pozadina u prirodnoj velicini unutar rc, uz gornju i levu ivicu
// ---------------------------------------------------------------
void CKol1View::DrawBackground(CDC* pDC, CRect rc)
{
	int w = min(m_pBackground->Width(), rc.Width());
	int h = min(m_pBackground->Height(), rc.Height());

	m_pBackground->Draw(pDC, CRect(0, 0, w, h),
		CRect(rc.left, rc.top, rc.left + w, rc.top + h));
}

// ---------------------------------------------------------------
// 4. Transparentno crtanje - transparentna je boja prvog piksela
// ---------------------------------------------------------------
void CKol1View::DrawImgTransparent(CDC* pDC, DImage* pImage)
{
	unsigned char* bits = pImage->GetDIBBits();
	COLORREF bgColor = RGB(bits[2], bits[1], bits[0]);   // u memoriji je BGR(A)

	CRect rc(0, 0, pImage->Width(), pImage->Height());
	pImage->DrawTransparent(pDC, rc, rc, bgColor);
}

// ---------------------------------------------------------------
// Hijerarhija (od zemlje navise):
//   leg2 -> leg1 -> body1 -> arm1 -> arm2
// Svaka Draw* funkcija pocinje u tacki kacenja (pivot je u (0,0)),
// rotira, pomeri sliku tako da njen pivot dodje u (0,0), nacrta je i
// ostavi k.s. u tacki gde se kaci sledeci deo.
// ---------------------------------------------------------------

// 9. Drugi deo noge (koren): rotacija oko (35,60) - R / F
//    na kraju k.s. ostaje u (35,60) gde se kaci leg1
void CKol1View::DrawLeg2(CDC* pDC)
{
	Rotate(pDC, (float)m_angleLeg2, false);
	Translate(pDC, -35.0f, -60.0f, false);
	DrawImgTransparent(pDC, m_pLeg2);
	Translate(pDC, 35.0f, 60.0f, false);
}

// 8. Prvi deo noge: rotacija oko (30,125) - Q / A
//    na kraju k.s. ostaje u (237,125) gde se kaci telo
void CKol1View::DrawLeg1(CDC* pDC)
{
	Rotate(pDC, (float)m_angleLeg1, false);
	Translate(pDC, -30.0f, -125.0f, false);
	DrawImgTransparent(pDC, m_pLeg1);
	Translate(pDC, 237.0f, 125.0f, false);
}

// 10. Telo: rotacija oko (26,133) - W / S
//     na kraju k.s. ostaje u tacki gde se kaci arm1
void CKol1View::DrawBody1(CDC* pDC)
{
	Rotate(pDC, (float)m_angleBody, false);
	Translate(pDC, -26.0f, -133.0f, false);
	DrawImgTransparent(pDC, m_pBody1);
	Translate(pDC, BODY_ARM_X, BODY_ARM_Y, false);
}

// 6. Prvi deo ruke: rotacija oko (34,31) - Q / A
//    k.s. se vraca (arm2 se crta pre arm1, pa sam racuna svoj polozaj)
void CKol1View::DrawArm1(CDC* pDC)
{
	XFORM oldForm;
	pDC->GetWorldTransform(&oldForm);

	Rotate(pDC, (float)m_angleArm1, false);
	Translate(pDC, -34.0f, -31.0f, false);
	DrawImgTransparent(pDC, m_pArm1);

	pDC->SetWorldTransform(&oldForm);
}

// 7. Drugi deo ruke: rotacija oko (23,61), poklapa se sa (210,102) na arm1 - T / G
//    Crta se PRE arm1 da bi u polozaju automobila bio sakriven iza arm1,
//    zato ovde ponavlja transformaciju arm1 da bi stigao do zgloba (210,102).
void CKol1View::DrawArm2(CDC* pDC)
{
	XFORM oldForm;
	pDC->GetWorldTransform(&oldForm);

	// transformacija arm1 (bez crtanja) -> zglob (210,102)
	Rotate(pDC, (float)m_angleArm1, false);
	Translate(pDC, -34.0f, -31.0f, false);
	Translate(pDC, 210.0f, 102.0f, false);

	// arm2
	Rotate(pDC, (float)m_angleArm2, false);
	Translate(pDC, -23.0f, -61.0f, false);
	DrawImgTransparent(pDC, m_pArm2);

	pDC->SetWorldTransform(&oldForm);
}

// ---------------------------------------------------------------
// 11. Ceo transformer
// ---------------------------------------------------------------
void CKol1View::DrawTransformer(CDC* pDC)
{
	CRect rcClient;
	GetClientRect(&rcClient);

	XFORM oldForm;
	pDC->GetWorldTransform(&oldForm);

	Translate(pDC, ROOT_X, (float)rcClient.Height() - ROOT_FROM_BOTTOM, false);

	DrawLeg2(pDC);    // k.s. -> zglob leg2/leg1
	DrawLeg1(pDC);    // k.s. -> zglob leg1/body1
	DrawBody1(pDC);   // k.s. -> zglob body1/arm1
	DrawArm2(pDC);    // prvo arm2 (iza)
	DrawArm1(pDC);    // pa arm1 (preko arm2)

	pDC->SetWorldTransform(&oldForm);
}

void CKol1View::DrawScene(CDC* pDC)
{
	CRect rcClient;
	GetClientRect(&rcClient);

	DrawBackground(pDC, rcClient);

	int oldMode = pDC->SetGraphicsMode(GM_ADVANCED);
	XFORM oldForm;
	pDC->GetWorldTransform(&oldForm);

	DrawTransformer(pDC);

	pDC->SetWorldTransform(&oldForm);
	pDC->SetGraphicsMode(oldMode);
}

// ---------------------------------------------------------------
// 2. Crtanje u memorijskom DC-ju, bez flicker-a
// ---------------------------------------------------------------
BOOL CKol1View::OnEraseBkgnd(CDC* pDC)
{
	return TRUE;   // ne brisi pozadinu - sve se crta preko memorijskog DC-a
}

void CKol1View::OnDraw(CDC* pDC)
{
	CKol1Doc* pDoc = GetDocument();
	ASSERT_VALID(pDoc);
	if (!pDoc)
		return;

	CRect rcClient;
	GetClientRect(&rcClient);

	CDC memDC;
	memDC.CreateCompatibleDC(pDC);
	CBitmap bmp;
	bmp.CreateCompatibleBitmap(pDC, rcClient.Width(), rcClient.Height());
	CBitmap* pOldBmp = memDC.SelectObject(&bmp);

	memDC.FillSolidRect(rcClient, RGB(255, 255, 255));

	DrawScene(&memDC);

	pDC->BitBlt(0, 0, rcClient.Width(), rcClient.Height(), &memDC, 0, 0, SRCCOPY);

	memDC.SelectObject(pOldBmp);
	bmp.DeleteObject();
	memDC.DeleteDC();
}

// ---------------------------------------------------------------
// 12. Jedan korak transformacije: ugao se pomera za target/N
//     i ogranicava na [automobil = 0, robot = target]
// ---------------------------------------------------------------
void CKol1View::StepAngle(double& angle, double target, int direction)
{
	angle += direction * target / TRANSFORM_STEPS;

	double lo = min(0.0, target);
	double hi = max(0.0, target);
	if (angle < lo) angle = lo;
	if (angle > hi) angle = hi;
}

void CKol1View::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	switch (nChar)
	{
	case 'Q': m_angleArm1 += STEP_ANGLE; m_angleLeg1 += STEP_ANGLE; break;
	case 'A': m_angleArm1 -= STEP_ANGLE; m_angleLeg1 -= STEP_ANGLE; break;
	case 'T': m_angleArm2 += STEP_ANGLE; break;
	case 'G': m_angleArm2 -= STEP_ANGLE; break;
	case 'R': m_angleLeg2 += STEP_ANGLE; break;
	case 'F': m_angleLeg2 -= STEP_ANGLE; break;
	case 'W': m_angleBody += STEP_ANGLE; break;
	case 'S': m_angleBody -= STEP_ANGLE; break;

	case VK_RIGHT:   // rasklapanje -> robot
	case VK_LEFT:    // sklapanje   -> automobil
	{
		int dir = (nChar == VK_RIGHT) ? 1 : -1;
		StepAngle(m_angleLeg2, ROBOT_LEG2, dir);
		StepAngle(m_angleLeg1, ROBOT_LEG1, dir);
		StepAngle(m_angleBody, ROBOT_BODY, dir);
		StepAngle(m_angleArm1, ROBOT_ARM1, dir);
		StepAngle(m_angleArm2, ROBOT_ARM2, dir);
		break;
	}

	default:
		CView::OnKeyDown(nChar, nRepCnt, nFlags);
		return;
	}

	Invalidate();
	CView::OnKeyDown(nChar, nRepCnt, nFlags);
}
