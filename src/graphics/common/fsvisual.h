#ifndef FSVISUAL_IS_INCLUDED
#define FSVISUAL_IS_INCLUDED
/* { */

#include <ysvisual.h>


const unsigned int FSVISUAL_DRAWOPAQUE=YsVisual::DRAWOPAQUE;
const unsigned int FSVISUAL_DRAWTRANSPARENT=YsVisual::DRAWTRANSPARENT;
const unsigned int FSVISUAL_DRAWALL=YsVisual::DRAWALL;


class FsVisualSrf : public YsVisualSrf
{
public:
	using YsShell::SetMatrix;
	using YsShell::ShootRayH;
	using YsShell::SetTrustPolygonNormal;

	inline FsVisualSrf(){}
	FsVisualSrf(const FsVisualSrf &incoming);
	FsVisualSrf &operator=(const FsVisualSrf &incoming);

	YSRESULT Load(const wchar_t fn[]);
};



class FsVisualDnm : public YsVisualDnm
{
public:
	FsVisualDnm();
	FsVisualDnm(std::nullptr_t);
	~FsVisualDnm();

	FsVisualDnm &operator=(std::nullptr_t)
	{
		dnmPtr=nullptr;
		return *this;
	}
	bool operator==(std::nullptr_t) const
	{
		return (nullptr==dnmPtr);
	}
	bool operator!=(std::nullptr_t) const
	{
		return (nullptr!=dnmPtr);
	}

	YSRESULT Load(const wchar_t fn[]);

};

inline bool operator==(std::nullptr_t,const FsVisualDnm &vis)
{
	return (vis==nullptr);
}
inline bool operator!=(std::nullptr_t,const FsVisualDnm &vis)
{
	return (vis!=nullptr);
}

////////////////////////////////////////////////////////////

/* } */
#endif
