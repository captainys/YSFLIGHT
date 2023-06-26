#define _CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES 1
#define _CRT_SECURE_CPP_OVERLOAD_STANDARD_NAMES_COUNT 1
#define _CRT_SECURE_NO_WARNINGS 1

#include <ysclass.h>
#include <stdlib.h>
#include "fs.h"


YsGLBufferManager::Handle FsExplosion::fireballVboHd=nullptr;
YsGLBufferManager::Handle FsExplosion::halfFireballVboHd=nullptr;
YsGLBufferManager::Handle FsExplosion::reverseFireballVboHd=nullptr;
YsGLBufferManager::Handle FsExplosion::reverseHalfFireballVboHd=nullptr;
YsGLBufferManager::Handle FsExplosion::waterPlumeVboHd=nullptr;


// Implementation //////////////////////////////////////////
FsExplosion::FsExplosion()
{
	timeRemain=0.0;
	timePassed=0.0;
	radius=0.0;

	next=NULL;
	prev=NULL;
}

void FsExplosion::Move(const double &dt)
{
	if(timeRemain>=YsTolerance)
	{
		timeRemain=timeRemain-dt;
		timePassed=timePassed+dt;
		if(timeRemain<YsTolerance)
		{
			timeRemain=0.0;
			timePassed=0.0;
		}
	}
}

// The followiwng function is moved to fsexplosionbi.cpp and fsexplosiongl.cpp
// void FsExplosion::Draw()

void FsExplosion::Explode(const YsVec3 &p,const double &remain,const double &iRad,const double &rad,YSBOOL f)
{
	pos=p;
	if(pos.y()<rad)
	{
		pos.SetY(0.0);
	}
	expType=FSEXPLOSION_FIREBALL;
	timeRemain=remain;
	timePassed=0.0;
	iniRadius=iRad;
	radius=rad;
	flash=f;

	random=rand();
}

void FsExplosion::WaterPlume(const YsVec3 &p,const double &remain,const double &rad,const double &hei)
{
	pos=p;
	expType=FSEXPLOSION_WATERPLUME;
	timeRemain=remain;
	timePassed=0.0;
	height=hei;
	radius=rad;
	flash=YSFALSE;

	random=rand();
}

//static 
const float FsExplosion::FireBallVtx[96*3]=
{
	 0.000000f, 0.000000f,-1.000000f, 0.707107f, 0.000000f,-0.707107f, 0.000000f, 0.707107f,-0.707107f,
	 0.707107f, 0.000000f,-0.707107f, 0.707107f, 0.707107f, 0.000000f, 0.000000f, 0.707107f,-0.707107f,
	 0.707107f, 0.000000f,-0.707107f, 1.000000f, 0.000000f, 0.000000f, 0.707107f, 0.707107f, 0.000000f,
	 0.000000f, 0.707107f,-0.707107f, 0.707107f, 0.707107f, 0.000000f, 0.000000f, 1.000000f, 0.000000f,
	 1.000000f, 0.000000f, 0.000000f, 0.707107f, 0.000000f, 0.707107f, 0.707107f, 0.707107f, 0.000000f,
	 0.707107f, 0.000000f, 0.707107f, 0.000000f, 0.707107f, 0.707107f, 0.707107f, 0.707107f, 0.000000f,
	 0.707107f, 0.000000f, 0.707107f, 0.000000f, 0.000000f, 1.000000f, 0.000000f, 0.707107f, 0.707107f,
	 0.707107f, 0.707107f, 0.000000f, 0.000000f, 0.707107f, 0.707107f, 0.000000f, 1.000000f, 0.000000f,
	 0.000000f, 0.000000f, 1.000000f,-0.707107f, 0.000000f, 0.707107f, 0.000000f, 0.707107f, 0.707107f,
	-0.707107f, 0.000000f, 0.707107f,-0.707107f, 0.707107f, 0.000000f, 0.000000f, 0.707107f, 0.707107f,
	-0.707107f, 0.000000f, 0.707107f,-1.000000f, 0.000000f, 0.000000f,-0.707107f, 0.707107f, 0.000000f,
	 0.000000f, 0.707107f, 0.707107f,-0.707107f, 0.707107f, 0.000000f, 0.000000f, 1.000000f, 0.000000f,
	-1.000000f, 0.000000f, 0.000000f,-0.707107f, 0.000000f,-0.707107f,-0.707107f, 0.707107f, 0.000000f,
	-0.707107f, 0.000000f,-0.707107f, 0.000000f, 0.707107f,-0.707107f,-0.707107f, 0.707107f, 0.000000f,
	-0.707107f, 0.000000f,-0.707107f, 0.000000f, 0.000000f,-1.000000f, 0.000000f, 0.707107f,-0.707107f,
	-0.707107f, 0.707107f, 0.000000f, 0.000000f, 0.707107f,-0.707107f, 0.000000f, 1.000000f, 0.000000f,
	 1.000000f, 0.000000f, 0.000000f, 0.707107f, 0.000000f,-0.707107f, 0.707107f,-0.707107f, 0.000000f,
	 0.707107f, 0.000000f,-0.707107f, 0.000000f,-0.707107f,-0.707107f, 0.707107f,-0.707107f, 0.000000f,
	 0.707107f, 0.000000f,-0.707107f, 0.000000f, 0.000000f,-1.000000f, 0.000000f,-0.707107f,-0.707107f,
	 0.707107f,-0.707107f, 0.000000f, 0.000000f,-0.707107f,-0.707107f, 0.000000f,-1.000000f, 0.000000f,
	 0.000000f, 0.000000f, 1.000000f, 0.707107f, 0.000000f, 0.707107f, 0.000000f,-0.707107f, 0.707107f,
	 0.707107f, 0.000000f, 0.707107f, 0.707107f,-0.707107f, 0.000000f, 0.000000f,-0.707107f, 0.707107f,
	 0.707107f, 0.000000f, 0.707107f, 1.000000f, 0.000000f, 0.000000f, 0.707107f,-0.707107f, 0.000000f,
	 0.000000f,-0.707107f, 0.707107f, 0.707107f,-0.707107f, 0.000000f, 0.000000f,-1.000000f, 0.000000f,
	-1.000000f, 0.000000f, 0.000000f,-0.707107f, 0.000000f, 0.707107f,-0.707107f,-0.707107f, 0.000000f,
	-0.707107f, 0.000000f, 0.707107f, 0.000000f,-0.707107f, 0.707107f,-0.707107f,-0.707107f, 0.000000f,
	-0.707107f, 0.000000f, 0.707107f, 0.000000f, 0.000000f, 1.000000f, 0.000000f,-0.707107f, 0.707107f,
	-0.707107f,-0.707107f, 0.000000f, 0.000000f,-0.707107f, 0.707107f, 0.000000f,-1.000000f, 0.000000f,
	 0.000000f, 0.000000f,-1.000000f,-0.707107f, 0.000000f,-0.707107f, 0.000000f,-0.707107f,-0.707107f,
	-0.707107f, 0.000000f,-0.707107f,-0.707107f,-0.707107f, 0.000000f, 0.000000f,-0.707107f,-0.707107f,
	-0.707107f, 0.000000f,-0.707107f,-1.000000f, 0.000000f, 0.000000f,-0.707107f,-0.707107f, 0.000000f,
	 0.000000f,-0.707107f,-0.707107f,-0.707107f,-0.707107f, 0.000000f, 0.000000f,-1.000000f, 0.000000f,
};

// static 
const float FsExplosion::Circle[17*3]=
{    
	 1.000000f,1.0f, 0.000000f,
	 0.923880f,0.5f,-0.382683f,
	 0.707107f,1.0f,-0.707107f,
	 0.382683f,0.5f,-0.923880f,
	 0.000000f,1.0f,-1.000000f,
	-0.382683f,0.5f,-0.923880f,
	-0.707107f,1.0f,-0.707107f,
	-0.923880f,0.5f,-0.382683f,
	-1.000000f,1.0f, 0.000000f,
	-0.923880f,0.5f, 0.382683f,
	-0.707107f,1.0f, 0.707107f,
	-0.382683f,0.5f, 0.923880f,
	-0.000000f,1.0f, 1.000000f,
	 0.382683f,0.5f, 0.923880f,
	 0.707107f,1.0f, 0.707107f,
	 0.923880f,0.5f, 0.382683f,
	 1.000000f,1.0f, 0.000000f
};

/* static */ void FsExplosion::PrepareFireballVertexArray(void)
{
	auto &bufMan=YsGLBufferManager::GetSharedBufferManager();

	if(nullptr==fireballVboHd)
	{
		fireballVboHd=bufMan.Create();
	}
	if(nullptr==halfFireballVboHd)
	{
		halfFireballVboHd=bufMan.Create();
	}
	if(nullptr==reverseFireballVboHd)
	{
		reverseFireballVboHd=bufMan.Create();
	}
	if(nullptr==reverseHalfFireballVboHd)
	{
		reverseHalfFireballVboHd=bufMan.Create();
	}

	YsGLVertexBuffer fullVtxBuf,halfVtxBuf,reverseFullVtxBuf,reverseHalfVtxBuf;
	YsGLNormalBuffer fullNomBuf,halfNomBuf,reverseFullNomBuf,reverseHalfNomBuf;

	int nHalfFireBall=0;
	for(int triIdx=0; triIdx<32; ++triIdx)
	{
		YSBOOL allPositiveY=YSTRUE;

		for(int vtIdx=0; vtIdx<3; ++vtIdx)
		{
			reverseFullVtxBuf.Add(FireBallVtx[(triIdx*3+vtIdx)*3  ],FireBallVtx[(triIdx*3+vtIdx)*3+1],FireBallVtx[(triIdx*3+vtIdx)*3+2]);
			reverseFullNomBuf.Add(FireBallVtx[(triIdx*3+vtIdx)*3  ],FireBallVtx[(triIdx*3+vtIdx)*3+1],FireBallVtx[(triIdx*3+vtIdx)*3+2]);

			fullVtxBuf.Add(FireBallVtx[(triIdx*3+2-vtIdx)*3  ],FireBallVtx[(triIdx*3+2-vtIdx)*3+1],FireBallVtx[(triIdx*3+2-vtIdx)*3+2]);
			fullNomBuf.Add(FireBallVtx[(triIdx*3+2-vtIdx)*3  ],FireBallVtx[(triIdx*3+2-vtIdx)*3+1],FireBallVtx[(triIdx*3+2-vtIdx)*3+2]);

			if(-YsTolerance>FireBallVtx[(triIdx*3+vtIdx)*3+1])
			{
				allPositiveY=YSFALSE;
			}
		}

		if(YSTRUE==allPositiveY)
		{
			for(int vtIdx=0; vtIdx<3; ++vtIdx)
			{
				reverseHalfVtxBuf.Add(FireBallVtx[(triIdx*3+vtIdx)*3  ],FireBallVtx[(triIdx*3+vtIdx)*3+1],FireBallVtx[(triIdx*3+vtIdx)*3+2]);
				reverseHalfNomBuf.Add(FireBallVtx[(triIdx*3+vtIdx)*3  ],FireBallVtx[(triIdx*3+vtIdx)*3+1],FireBallVtx[(triIdx*3+vtIdx)*3+2]);

				halfVtxBuf.Add(FireBallVtx[(triIdx*3+2-vtIdx)*3  ],FireBallVtx[(triIdx*3+2-vtIdx)*3+1],FireBallVtx[(triIdx*3+2-vtIdx)*3+2]);
				halfNomBuf.Add(FireBallVtx[(triIdx*3+2-vtIdx)*3  ],FireBallVtx[(triIdx*3+2-vtIdx)*3+1],FireBallVtx[(triIdx*3+2-vtIdx)*3+2]);
			}
			++nHalfFireBall;
		}
	}

	if(16!=nHalfFireBall)
	{
		printf("Boom! %d\n",nHalfFireBall);
		printf("%s %d\n",__FUNCTION__,__LINE__);
		exit(1);
	}

	if(nullptr!=fireballVboHd)
	{
		bufMan.MakeVtxNom(fireballVboHd,YsGL::TRIANGLES,fullVtxBuf,fullNomBuf);
	}
	if(nullptr!=halfFireballVboHd)
	{
		bufMan.MakeVtxNom(halfFireballVboHd,YsGL::TRIANGLES,halfVtxBuf,halfNomBuf);
	}
	if(nullptr!=reverseFireballVboHd)
	{
		bufMan.MakeVtxNom(reverseFireballVboHd,YsGL::TRIANGLES,reverseFullVtxBuf,reverseFullNomBuf);
	}
	if(nullptr!=reverseHalfFireballVboHd)
	{
		bufMan.MakeVtxNom(reverseHalfFireballVboHd,YsGL::TRIANGLES,reverseHalfVtxBuf,reverseHalfNomBuf);
	}
}

/* static */ void FsExplosion::PrepareWaterPlumeVertexArray(void)
{
	auto &bufMan=YsGLBufferManager::GetSharedBufferManager();

	if(nullptr==waterPlumeVboHd)
	{
		waterPlumeVboHd=bufMan.Create();
	}

	if(nullptr!=waterPlumeVboHd)
	{
		const int maxNVtx=34*2;
		YsGLVertexBuffer vtxBuf;
		YsGLNormalBuffer nomBuf;

		for(int i=0; i<=16; i++)
		{
			nomBuf.Add(Circle[i*3+0],0.0f,Circle[i*3+2]);
			vtxBuf.Add(Circle[i*3+0],0.0f,Circle[i*3+2]);

			nomBuf.Add(Circle[i*3+0],0.0f,Circle[i*3+2]);
			vtxBuf.Add(Circle[i*3+0],Circle[i*3+1],Circle[i*3+2]);
		}

		for(int i=0; i<=16; i++)
		{
			nomBuf.Add(Circle[i*3+0],0.0f,Circle[i*3+2]);
			vtxBuf.Add(Circle[i*3+0],Circle[i*3+1],Circle[i*3+2]);

			nomBuf.Add(Circle[i*3+0],0.0f,Circle[i*3+2]);
			vtxBuf.Add(Circle[i*3+0],0.0f,Circle[i*3+2]);
		}

		bufMan.MakeVtxNom(waterPlumeVboHd,YsGL::TRIANGLES,vtxBuf,nomBuf);
	}
}

// Implementation //////////////////////////////////////////
FsExplosionHolder::FsExplosionHolder()
{
	toPlay=NULL;
	toSave=new FsRecord <FsExplosionRecord>;

	activeList=NULL;
	freeList=NULL;

	Clear();

	netServer=NULL;
	netClient=NULL;
}

FsExplosionHolder::~FsExplosionHolder()
{
	if(toSave!=NULL)
	{
		delete toSave;
	}
	if(toPlay!=NULL)
	{
		delete toPlay;
	}
}

YSRESULT FsExplosionHolder::Save(FILE *fp,FsSimulation *sim)
{
	if(toPlay!=NULL)
	{
		int i,nRec;
		nRec=toPlay->GetNumRecord();
		fprintf(fp,"EXPRECOR\n");

		fprintf(fp,"VERSION 3\n");  // Version 2: Flash is added.
		                            // Version 3: Explosion Type is added.
		fprintf(fp,"NUMRECO %d\n",nRec);

		FsExplosionRecord *rec;
		for(i=0; i<nRec; i++)
		{
			int ownerId;
			double t;

			rec=toPlay->GetElement(t,i);

			fprintf(fp,"%g %.2f %.2f %.2f %.2f %.2f %.2f ",
			    t,rec->x,rec->y,rec->z,rec->remain,rec->iniRadius,rec->radius);

			if((ownerId=sim->GetAirplaneIdFromHandle(rec->causedBy))>=0)
			{
				fprintf(fp,"A%d ",ownerId);
			}
			else //if((ownerId=sim->GetGroundObjectId(rec->causedBy))>=0)
			//{
			//	fprintf(fp,"G%d ",ownerId);
			//}
			//else
			{
				fprintf(fp,"N ");
			}

			fprintf(fp,"%d ",rec->flash);
			fprintf(fp,"%d\n",rec->expType);
		}
		fprintf(fp,"ENDRECO\n");
	}
	return YSOK;
}

YSRESULT FsExplosionHolder::Load(FILE *fp,FsSimulation *sim)
{
	if(toSave!=NULL)
	{
		delete toSave;
		toSave=NULL;
	}
	if(toPlay!=NULL)
	{
		delete toPlay;
		toPlay=NULL;
	}

	toPlay=new FsRecord <FsExplosionRecord>;

	FsExplosionRecord rec;
	char buf[256];
	int expVersion;
	expVersion=1;
	while(fgets(buf,256,fp)!=NULL)
	{
		if(strncmp(buf,"VERSION 1",9)==0)
		{
			expVersion=1;
		}
		else if(strncmp(buf,"VERSION 2",9)==0)
		{
			expVersion=2;
		}
		else if(strncmp(buf,"VERSION 3",9)==0)
		{
			expVersion=3;
		}
		else if(strncmp(buf,"NUMRECO",7)==0)
		{
			int i,nRec;
			nRec=atoi(buf+8);
			for(i=0; i<nRec; i++)
			{
				char str[256];
				double t;

				fgets(buf,256,fp);
				if(expVersion==3)
				{
					sscanf(buf,"%lf%f%f%f%f%f%f%s%d%d",
						   &t,&rec.x,&rec.y,&rec.z,
						   &rec.remain,&rec.iniRadius,&rec.radius,str,(int *)&rec.flash,&rec.expType);
				}
				else if(expVersion==2)
				{
					sscanf(buf,"%lf%f%f%f%f%f%f%s%d",
						   &t,&rec.x,&rec.y,&rec.z,
						   &rec.remain,&rec.iniRadius,&rec.radius,str,(int *)&rec.flash);
					rec.expType=FSEXPLOSION_FIREBALL;
				}
				else // Version 1
				{
					sscanf(buf,"%lf%f%f%f%f%f%f%s",
					    &t,&rec.x,&rec.y,&rec.z,
					    &rec.remain,&rec.iniRadius,&rec.radius,str);
					rec.flash=YSFALSE;
					rec.expType=FSEXPLOSION_FIREBALL;
				}

				rec.causedBy=NULL;
				if(str[0]=='A')
				{
					rec.causedBy=sim->GetAirplaneById(atoi(str+1));
				}
				if(str[0]=='G')
				{
					//rec.causedBy=sim->GetGroundObject(atoi(str+1));
				}
				toPlay->AddElement(rec,t);
			}
		}
		else if(strncmp(buf,"ENDRECO",7)==0)
		{
			break;
		}
		else
		{
			fsStderr.Printf("Unrecognized Explosion Record\n");
		}
	}

	return YSOK;
}

void FsExplosionHolder::Clear(void)
{
	int i;
	for(i=0; i<NumExplosion; i++)
	{
		buf[i].timeRemain=0.0;

		if(i==0)
		{
			buf[i].prev=NULL;
			buf[i].next=&buf[i+1];
		}
		else if(i==NumExplosion-1)
		{
			buf[i].prev=&buf[i-1];
			buf[i].next=NULL;
		}
		else
		{
			buf[i].prev=&buf[i-1];
			buf[i].next=&buf[i+1];
		}
	}

	activeList=NULL;
	freeList=&buf[0];
}

void FsExplosionHolder::MoveToActiveList(FsExplosion *wep)
{
	FsExplosion *next,*prev;

	next=wep->next;
	prev=wep->prev;
	if(next!=NULL)
	{
		next->prev=prev;
	}
	if(prev!=NULL)
	{
		prev->next=next;
	}
	else
	{
		freeList=next;
	}

	if(activeList!=NULL)
	{
		activeList->prev=wep;
	}
	wep->next=activeList;
	wep->prev=NULL;
	activeList=wep;
}

void FsExplosionHolder::MoveToFreeList(FsExplosion *wep)
{
	FsExplosion *next,*prev;

	next=wep->next;
	prev=wep->prev;
	if(next!=NULL)
	{
		next->prev=prev;
	}
	if(prev!=NULL)
	{
		prev->next=next;
	}
	else
	{
		activeList=next;
	}

	if(freeList!=NULL)
	{
		freeList->prev=wep;
	}
	wep->next=freeList;
	wep->prev=NULL;
	freeList=wep;
}

YSRESULT FsExplosionHolder::RipOffEarlyPartOfRecord(void)
{
	if(toSave!=NULL)
	{
		toSave->RipOffEarlyPartOfRecord();
	}
	return YSOK;
}

YSRESULT FsExplosionHolder::Explode(
    const double &ctime,
    const YsVec3 &pos,
    const double &remain,
    const double &iniRadius,
    const double &radius,
    YSBOOL flash,
    FsExistence *owner,
    YSBOOL recordIt)
{
	FsExplosion *toExplode;

	toExplode=freeList;
	if(toExplode!=NULL)
	{
		toExplode->Explode(pos,remain,iniRadius,radius,flash);

		MoveToActiveList(toExplode);

		if(recordIt==YSTRUE && toSave!=NULL)
		{
			FsExplosionRecord rec;
			rec.expType=FSEXPLOSION_FIREBALL;
			rec.x=float(pos.x());
			rec.y=float(pos.y());
			rec.z=float(pos.z());
			rec.remain=float(remain);
			rec.iniRadius=float(iniRadius);
			rec.radius=float(radius);
			rec.flash=flash;
			rec.causedBy=owner;
			toSave->AddElement(rec,ctime);
		}
		return YSOK;
	}
	return YSERR;
}

YSRESULT FsExplosionHolder::WaterPlume(
    const double &ctime,
    const YsVec3 &pos,
    const double &remain,
    const double &radius,
    const double &height,
    class FsExistence *owner,
    YSBOOL recordIt)
{
	FsExplosion *toExplode;

	toExplode=freeList;
	if(toExplode!=NULL)
	{
		toExplode->WaterPlume(pos,remain,radius,height);

		MoveToActiveList(toExplode);

		if(recordIt==YSTRUE && toSave!=NULL)
		{
			FsExplosionRecord rec;
			rec.expType=FSEXPLOSION_WATERPLUME;
			rec.x=float(pos.x());
			rec.y=float(pos.y());
			rec.z=float(pos.z());
			rec.remain=float(remain);
			rec.iniRadius=float(height);
			rec.radius=float(radius);
			rec.flash=YSFALSE;
			rec.causedBy=owner;
			toSave->AddElement(rec,ctime);
		}
		return YSOK;
	}
	return YSERR;
}

void FsExplosionHolder::Move(const double &dt)
{
	FsExplosion *seeker,*nxt;
	for(seeker=activeList; seeker!=NULL; seeker=nxt)
	{
		nxt=seeker->next;

		seeker->Move(dt);
		if(seeker->timeRemain<=YsTolerance)
		{
			MoveToFreeList(seeker);
		}
	}
}

// The followiwng function is moved to fsexplosionbi.cpp and fsexplosiongl.cpp
// void FsExplosionHolder::Draw() const

void FsExplosionHolder::CollectRecord(void)
{
	FsRecord <FsExplosionRecord> *neo;

	neo=new FsRecord <FsExplosionRecord>;
	//fsConsole.Printf("  A\n");
	if(neo!=NULL)
	{
		int nToSave,nToPlay;
		nToSave=(toSave!=NULL ? toSave->GetNumRecord() : 0);
		nToPlay=(toPlay!=NULL ? toPlay->GetNumRecord() : 0);

		int i,j;
		i=0;
		j=0;
		//fsConsole.Printf("  B\n");
		while(i<nToSave || j<nToPlay)
		{
			FsExplosionRecord *s,*p;
			double st,pt;

			s=(toSave!=NULL ? toSave->GetElement(st,i) : NULL);
			p=(toPlay!=NULL ? toPlay->GetElement(pt,j) : NULL);

			if(s!=NULL && p==NULL)
			{
				neo->AddElement(*s,st);
				i++;
			}
			else if(s==NULL && p!=NULL)
			{
				neo->AddElement(*p,pt);
				j++;
			}
			else if(st<pt)
			{
				neo->AddElement(*s,st);
				i++;
			}
			else
			{
				neo->AddElement(*p,pt);
				j++;
			}

			if(p==NULL && s==NULL)
			{
				fsStderr.Printf("Something Unexpected Happens.\n");
				break;
			}
		}
		//fsConsole.Printf("  C\n");

		if(toSave!=NULL)
		{
			delete toSave;
			toSave=NULL;
		}
		if(toPlay!=NULL)
		{
			delete toPlay;
			toPlay=NULL;
		}

		//fsConsole.Printf("  D\n");
		toSave=new FsRecord <FsExplosionRecord>;;
		toPlay=neo;
	}
	//fsConsole.Printf("  E\n");
}

void FsExplosionHolder::PlayRecord(const double &t1,const double &dt)
{
	YSSIZE_T i,i1,i2;
	double t2,t;
	t2=t1+dt;

	if(toPlay!=NULL)
	{
		if(toPlay->GetIndexByTime(i1,i2,t1,t2)==YSOK)
		{
			for(i=i1; i<=i2; i++)
			{
				FsExplosionRecord *rec;
				rec=toPlay->GetElement(t,i);

				if(rec!=NULL)
				{
					YsVec3 pos;
					pos.Set(rec->x,rec->y,rec->z);
					switch(rec->expType)
					{
					case FSEXPLOSION_FIREBALL:
						Explode(t,
						        pos,
							    rec->remain,
							    rec->iniRadius,
							    rec->radius,
							    rec->flash,
							    rec->causedBy,
							    YSFALSE);
						break;
					case FSEXPLOSION_WATERPLUME:
						WaterPlume(t,
						           pos,
						           rec->remain,
						           rec->radius,
						           rec->iniRadius,
						           rec->causedBy,
						           YSFALSE);
						break;
					}
				}
			}
		}
	}
}


void FsExplosionHolder::SetNetServer(class FsSocketServer *svr)
{
	netServer=svr;
}

void FsExplosionHolder::SetNetClient(class FsSocketClient *cli)
{
	netClient=cli;
}

