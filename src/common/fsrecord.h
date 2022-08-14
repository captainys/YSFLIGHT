#ifndef FSRECORD_IS_INCLUDED
#define FSRECORD_IS_INCLUDED
/* { */

////////////////////////////////////////////////////////////

template <class T>
class FsAllocOnceArray
{
public:
	inline FsAllocOnceArray();
	inline ~FsAllocOnceArray();
	inline YSRESULT Alloc(int n);

	inline FsAllocOnceArray <T> &operator=(const FsAllocOnceArray <T> &from);
	inline const T &operator[](int idx) const;
	inline T &operator[](int idx);

	inline int GetN(void) const;

protected:
	int lng;
	T *dat;
};

template <class T>
inline FsAllocOnceArray <T>::FsAllocOnceArray()
{
	lng=0;
	dat=NULL;
}

template <class T>
inline FsAllocOnceArray <T>::~FsAllocOnceArray()
{
	if(dat!=NULL)
	{
		delete [] dat;
	}
}

template <class T>
inline YSRESULT FsAllocOnceArray <T>::Alloc(int n)
{
	if(dat!=NULL)
	{
		delete [] dat;
		dat=NULL;
		lng=0;
	}

	dat=new T [n];
	if(dat!=NULL)
	{
		lng=n;
		return YSOK;
	}

	return YSERR;
}

template <class T>
inline FsAllocOnceArray <T> &FsAllocOnceArray <T>::operator=(const FsAllocOnceArray <T> &from)
{
	int i;

	if(dat!=NULL)
	{
		delete [] dat;
		dat=NULL;
		lng=0;
	}

	if(from.lng>0)
	{
		dat=new T [from.lng];
		lng=from.lng;
		for(i=0; i<from.lng; i++)
		{
			dat[i]=from.dat[i];
		}
	}

	return *this;
}

template <class T>
inline const T &FsAllocOnceArray <T>::operator [](int idx) const
{
	return dat[idx];
}

template <class T>
inline T &FsAllocOnceArray <T>::operator [](int idx)
{
	return dat[idx];
}

template <class T>
inline int FsAllocOnceArray <T>::GetN(void) const
{
	return lng;
}



////////////////////////////////////////////////////////////


template <class T> class FsRecordElement
{
public:
	double t;
	T dat;
};

// The word:
//  "Index" is a serial number of element. (0 through infinity)
//  "Offset" is offset from FsRecordBlock::dat[0] (0 through FsNumElemBlock-1)

template <class T> class FsRecord
{
public:
	enum 
	{
		SEGARRAY_BITSHIFT=10
	};

	FsRecord();
	virtual ~FsRecord();

	YSRESULT AddElement(T &dat,double t);
	YSRESULT GetIndexByTime(YSSIZE_T &idx,double t);
	YSRESULT GetIndexByTime(YSSIZE_T &idx1,YSSIZE_T &idx2,double t1,double t2);  // It is more like GetIndexRangeByTimeRange t1-t2 must enclose idx1-idx2
	T *GetElement(double &t,YSSIZE_T index);
	const T *GetElement(double &t,YSSIZE_T index) const;
	int GetNumRecord(void);

	T *GetTopElement(double &t);
	const T *GetTopElement(double &t) const;
	T *GetLastElement(double &t);
	const T *GetLastElement(double &t) const;


	YSRESULT RipOffEarlyPartOfRecord(void);
	YSRESULT DeleteRecord(const double &t1,const double &t2);

	const double GetRecordBeginTime(void) const;
	const double GetRecordEndTime(void) const;

protected:
	YsSegmentedArray <FsRecordElement <T>,SEGARRAY_BITSHIFT> recordArray;
};

template <class T> FsRecord <T>::FsRecord()
{
}

template <class T> FsRecord <T>::~FsRecord()
{
}

template <class T> YSRESULT FsRecord <T>::AddElement(T &dat,double t)
{
	recordArray.Increment();
	recordArray.GetEnd().dat=dat;
	recordArray.GetEnd().t=t;
	return YSOK;
}

template <class T>
YSRESULT FsRecord <T>::GetIndexByTime(YSSIZE_T &idx,double t)
{
	if(1==recordArray.GetN())
	{
		if(recordArray[0].t<t)
		{
			idx=0;
			return YSOK;
		}
	}
	else if(2<=recordArray.GetN())
	{
		if(recordArray[0].t<=t && t<recordArray.GetEnd().t)
		{
			YSSIZE_T i1=0,i2=recordArray.GetN()-1;
			while(1<YsAbs(i1-i2))
			{
				const YSSIZE_T im=(i1+i2)/2;
				if(t<recordArray[im].t)
				{
					i2=im;
				}
				else
				{
					i1=im;
				}
			}
			idx=i1;
			return YSOK;
		}
	}

	idx=-1;
	return YSERR;
}

template <class T> 
YSRESULT FsRecord <T>::GetIndexByTime(YSSIZE_T &idx1,YSSIZE_T &idx2,double t1,double t2)
{
	idx1=-1;
	idx2=-1;

	if(0<recordArray.GetN())
	{
		if(t1<recordArray[0].t)
		{
			idx1=0;
		}
		else if(t1<=recordArray.GetEnd().t)
		{
			YSSIZE_T i1,i2;
			i1=0;
			i2=recordArray.GetN()-1;
			while(YsAbs(i1-i2)>1)
			{
				const YSSIZE_T im=(i1+i2)/2;
				if(recordArray[im].t<t1)
				{
					i1=im;
				}
				else
				{
					i2=im;
				}
			}
			if(i1==i2)
			{
				idx1=i1+1;
			}
			else
			{
				idx1=i2;
			}
		}

		if(recordArray.GetEnd().t<=t2)
		{
			idx2=recordArray.GetN()-1;
		}
		else if(recordArray[0].t<t2 && t2<recordArray.GetEnd().t)
		{
			YSSIZE_T i1,i2;
			i1=0;
			i2=recordArray.GetN()-1;
			while(YsAbs(i1-i2)>1)
			{
				const YSSIZE_T im=(i1+i2)/2;
				if(t2<recordArray[im].t)
				{
					i2=im;
				}
				else
				{
					i1=im;
				}
			}
			idx2=i1;
		}
	}

	if(idx1>=0 && idx2>=0 && idx1<=idx2)
	{
		return YSOK;
	}
	else
	{
		idx1=-1;
		idx2=-1;
		return YSERR;
	}
}

template <class T>
T *FsRecord <T>::GetElement(double &t,YSSIZE_T idx)
{
	if(YSTRUE==recordArray.IsInRange(idx))
	{
		t=recordArray[idx].t;
		return &recordArray[idx].dat;
	}
	t=0.0;
	return NULL;
}

template <class T>
const T *FsRecord <T>::GetElement(double &t,YSSIZE_T idx) const
{
	if(YSTRUE==recordArray.IsInRange(idx))
	{
		t=recordArray[idx].t;
		return &recordArray[idx].dat;
	}
	t=0.0;
	return NULL;
}

template <class T>
int FsRecord<T>::GetNumRecord(void)
{
	return (int)recordArray.GetN();
}


template <class T>
T *FsRecord<T>::GetTopElement(double &t)
{
	if(0<recordArray.GetN())
	{
		t=recordArray[0].t;
		return &recordArray[0].dat;
	}
	t=0.0;
	return NULL;
}

template <class T> 
const T *FsRecord<T>::GetTopElement(double &t) const
{
	if(0<recordArray.GetN())
	{
		t=recordArray[0].t;
		return &recordArray[0].dat;
	}
	t=0.0;
	return NULL;
}

template <class T>
T *FsRecord<T>::GetLastElement(double &t)
{
	if(0<recordArray.GetN())
	{
		t=recordArray.GetEnd().t;
		return &recordArray.GetEnd().dat;
	}
	t=0.0;
	return NULL;
}

template <class T>
const T *FsRecord<T>::GetLastElement(double &t) const
{
	if(0<recordArray.GetN())
	{
		t=recordArray.GetEnd().t;
		return &recordArray.GetEnd().dat;
	}
	t=0.0;
	return NULL;
}

template <class T> YSRESULT FsRecord <T>::RipOffEarlyPartOfRecord(void)
{
	const YSSIZE_T nSave=(2<<SEGARRAY_BITSHIFT);

	if(nSave<recordArray.GetN())
	{
		const YSSIZE_T offset=recordArray.GetN()-nSave;
		for(YSSIZE_T idx=0; idx<nSave; ++idx)
		{
			recordArray[idx]=recordArray[idx+offset];
		}
		recordArray.Resize(nSave);
	}
	return YSOK;
}

template <class T> YSRESULT FsRecord <T>::DeleteRecord(const double &t1,const double &t2)
{
	if(t2<t1)
	{
		return DeleteRecord(t2,t1);
	}

	double topT,lastT;
	if(NULL!=GetTopElement(topT) && NULL!=GetLastElement(lastT))
	{
		if(lastT<=t1)               //                                       (t1)<--Del-->(t2)
		{                           //        (topT)<---Record--->(lastT)
			return YSOK;
		}
		else if(t2<=topT)           //  (t1)<--Del-->(t2)
		{                           //                      (topT)<---Record--->(lastT)
			const double dt=t2-t1;
			for(YSSIZE_T recIdx=0; recIdx<recordArray.GetN(); ++recIdx)
			{
				recordArray[recIdx].t-=dt;
			}
			return YSOK;
		}
		else if(t1<=topT && lastT<=t2)  //  (t1)<-------------Del------------------->(t2)
		{                               //           (topT)<---Record--->(lastT)
			recordArray[0].t=t1;
			recordArray.Resize(1);
			return YSOK;
		}
		else
		{
			const double t[2]={t1,t2};
			YSSIZE_T idx[2]={-1,-1};

			for(YSSIZE_T i=0; i<2; i++)
			{
				if(t[i]<=recordArray[0].t)
				{
					idx[i]=0;
				}
				else if(recordArray[0].t<=t[i] && t[i]<=recordArray.GetEnd().t)
				{
					YSSIZE_T i1,i2;  // Go bisection
					i1=0;
					i2=recordArray.GetN()-1;
					while(i2-i1>1)
					{
						const YSSIZE_T mid=(i1+i2)/2;
						if(recordArray[mid].t<=t[i])
						{
							i1=mid;
						}
						else
						{
							i2=mid;
						}
					}
					idx[i]=i1;
				}
				else
				{
					idx[i]=recordArray.GetN()-1;
				}
			}

			YSSIZE_T beginIdx=idx[0];
			YSSIZE_T endIdx=idx[1];

			if(topT<t1 && lastT<=t2)  //             (t1)<----------Del------------>(t2)
			{                         //  (topT)<------------Record--->(lastT)
				recordArray.Resize(beginIdx);
			}
			else
			{
				const double dt=t2-t1;
				//           (t1)<--Del-->(t2)
				//  (topT)<-------Record-------->(lastT)
				//  (t1)<--Del-------->(t2)
				//           (topT)<---Record--->(lastT)

				const YSSIZE_T nDel=endIdx-beginIdx;
				if(0<nDel)
				{
					for(YSSIZE_T recIdx=beginIdx; recIdx<recordArray.GetN()-nDel; ++recIdx)
					{
						recordArray[recIdx]=recordArray[recIdx+nDel];
						recordArray[recIdx].t-=dt;
					}
					recordArray.Resize(recordArray.GetN()-nDel);
				}
				else // Even if none is deletd, time stamps must be updated.
				{
					for(YSSIZE_T recIdx=beginIdx; recIdx<recordArray.GetN()-nDel; ++recIdx)
					{
						recordArray[recIdx].t-=dt;
					}
				}
			}
		}
	}
	return YSOK;
}

template <class T>
const double FsRecord <T>::GetRecordBeginTime(void) const
{
	if(0<recordArray.GetN())
	{
		return recordArray[0].t;
	}
	else
	{
		return YsInfinity;
	}
}

template <class T>
const double FsRecord <T>::GetRecordEndTime(void) const
{
	if(0<recordArray.GetN())
	{
		return recordArray.GetEnd().t;
	}
	return 0.0;
}

class FsTurretRecord
{
public:
	float h,p;
	unsigned int turretState;
};

class FsFlightRecord
{
public:
	enum
	{
		FLAGS_AB=1,
		FLAGS_ILS=2,
		FLAGS_VECTOR=4,
		FLAGS_OUTOFRUNWAY=8,
		FLAGS_BEACON=16,
		FLAGS_NAVLIGHT=32,
		FLAGS_STROBE=64,
		FLAGS_LANDINGLIGHT=128
	};
	YsVec3 pos;
	float h,p,b;
	float g;
	unsigned char state,vgw,spoiler,gear,flap,brake,smoke,vapor;
	unsigned short flags;
	unsigned char dmgTolerance;
	unsigned char thr;
	char elv,ail,rud,elvTrim;
	unsigned char thrVector,bombBay,thrReverser;  // 2003/04/11
	FsAllocOnceArray <FsTurretRecord> turret;
};

inline int operator==(const FsFlightRecord &a,const FsFlightRecord &b)
{
	return (&a!=NULL &&
	        &b!=NULL &&
	        a.pos==b.pos &&
	        fabs(a.h-b.h)<YsTolerance &&
	        fabs(a.p-b.p)<YsTolerance &&
	        fabs(a.b-b.b)<YsTolerance &&
	        fabs(a.g-b.g)<YsTolerance &&
	        a.state==b.state &&
			a.vgw==b.vgw &&
			a.spoiler==b.spoiler &&
			a.gear==b.gear &&
			a.flap==b.flap &&
			a.brake==b.brake &&
			a.smoke==b.smoke &&
			a.vapor==b.vapor &&
			a.flags==b.flags &&
			a.dmgTolerance==b.dmgTolerance &&
			a.thr==b.thr &&
			a.elv==b.elv &&
			a.ail==b.ail &&
			a.rud==b.rud &&
			a.elvTrim==b.elvTrim &&
			a.thrVector==b.thrVector &&
			a.bombBay==b.bombBay &&
			a.thrReverser==b.thrReverser);
}

inline int operator!=(const FsFlightRecord &a,const FsFlightRecord &b)
{
	return !(a==b);
}

class FsGroundRecord
{
public:
	YsVec3 pos;
	float h,p,b;
	unsigned char state,dmgTolerance;
	char steering;
	unsigned char leftDoor,rightDoor,rearDoor,brake;
	unsigned char lightState;
	float aaaAimh,aaaAimp,aaaAimb;
	float samAimh,samAimp,samAimb;
	float canAimh,canAimp,canAimb;
	FsAllocOnceArray <FsTurretRecord> turret;
};

inline int operator==(const FsGroundRecord &a,const FsGroundRecord &b)
{
	return (&a!=NULL &&
	        &b!=NULL &&
	        a.pos==b.pos &&
	        fabs(a.h-b.h)<YsTolerance &&
	        fabs(a.p-b.p)<YsTolerance &&
	        fabs(a.b-b.b)<YsTolerance &&
	        a.steering==b.steering &&
	        a.leftDoor==b.leftDoor &&
	        a.rightDoor==b.rightDoor &&
	        a.rearDoor==b.rearDoor &&
	        a.brake==b.brake &&
	        a.lightState==b.lightState &&
	        a.state==b.state &&
	        a.dmgTolerance==b.dmgTolerance &&
	        YsAbs(a.aaaAimh-b.aaaAimh)<YsTolerance &&
	        YsAbs(a.aaaAimp-b.aaaAimp)<YsTolerance &&
	        YsAbs(a.aaaAimb-b.aaaAimb)<YsTolerance &&
	        YsAbs(a.samAimh-b.samAimh)<YsTolerance &&
	        YsAbs(a.samAimp-b.samAimp)<YsTolerance &&
	        YsAbs(a.samAimb-b.samAimb)<YsTolerance &&
	        YsAbs(a.canAimh-b.canAimh)<YsTolerance &&
	        YsAbs(a.canAimp-b.canAimp)<YsTolerance &&
	        YsAbs(a.canAimb-b.canAimb)<YsTolerance);
}

inline int operator!=(const FsGroundRecord &a,const FsGroundRecord &b)
{
	return !(a==b);
}

/* } */
#endif
