#include <ysclass.h>
#include <ysglparticlemanager.h>

#include "fstexturemanager.h"
#include "fsparticle.h"




void FsParticle::Initialize(YsListItem <FsParticle> *ptr)
{
	exist=YSFALSE;
	thisInTheList=ptr;
}

void FsParticle::Create(const YsVec3 &pos,const YsVec3 &vel,const double &drag,const double &timeRemain,const double &elevation)
{
	this->exist=YSTRUE;
	this->pos=pos;
	this->vel=vel;
	this->drag=drag;
	this->timeRemain=timeRemain;
	this->timePassed=0.0;
	this->elevation=elevation;
	this->pattern=rand()%8;
	this->curSize=3.0;
	this->maxSize=3.0;
	this->fadeTime=1.0;
	this->growthRate=0.0;

	this->gravity.Set(0.0,-9.8,0.0);
	this->col=YsBlack();
	this->colTrans=FSPARTICLECOLOR_STATIC;
}

void FsParticle::SetGravity(const YsVec3 &gravity)
{
	this->gravity=gravity;
}

void FsParticle::SetColor(const YsColor &col)
{
	this->col=col;
}

void FsParticle::SetColorTransition(FSPARTICLECOLORTRANSITION colTrans)
{
	this->colTrans=colTrans;
}

void FsParticle::Move(const double &dt,const YsVec3 &wind)
{
	if(exist==YSTRUE)
	{
		timeRemain-=dt;
		if(timeRemain>0.0)
		{
			timePassed+=dt;

			switch(colTrans)
			{
			case FSPARTICLECOLOR_BURN:
				if(timePassed<0.5)
				{
					// 0.0: White -> 0.5:Yellow
					double b=1.0-timePassed*2.0;
					col.SetDoubleRGB(1.0,1.0,b);
				}
				else if(timePassed<1.0)
				{
					// 0.5:Yellow -> 1.0 Red
					double g=2.0-timePassed*2.0;
					col.SetDoubleRGB(1.0,g,0.0);
				}
				else if(timePassed<1.5)
				{
					// 1.0:Red -> 1.5:Black
					double r=3.0-timePassed*2.0;
					col.SetDoubleRGB(r,0.0,0.0);
				}
				break;
			default:
				break;
			}

			vel-=wind;
			vel*=(1.0-drag*dt);
			vel+=gravity*dt;
			vel+=wind;

			if(curSize<maxSize)
			{
				curSize+=growthRate*dt;
			}

			pos+=vel*dt;
			if(pos.y()<elevation)
			{
				exist=YSFALSE;
			}
		}
		else
		{
			exist=YSFALSE;
		}
	}
}

const double FsParticle::TimeRemain(void) const
{
	return timeRemain;
}

YSBOOL FsParticle::IsAlive(void) const
{
	return exist;
}

const YsColor &FsParticle::GetColor(void) const
{
	return col;
}

int FsParticle::GetPattern(void) const
{
	return pattern;
}

void FsParticle::SetSize(double curSize,double maxSize,double growthRate)
{
	this->curSize=curSize;
	this->maxSize=maxSize;
	this->growthRate=growthRate;
}

void FsParticle::SetSize(double curSize)
{
	this->curSize=curSize;
	this->maxSize=curSize;
	this->growthRate=0.0;
}
double FsParticle::GetSize(void) const
{
	return curSize;
}

double FsParticle::GetAlphaScale(void) const
{
	if(timeRemain<fadeTime)
	{
		return timeRemain/fadeTime;
	}
	return 1.0;
}

const YsVec3 &FsParticle::GetPosition(void) const
{
	return pos;
}



////////////////////////////////////////////////////////////


void FsParticleGenerator::Initialize(YsListItem <FsParticleGenerator> *ptr)
{
	thisInTheList=ptr;
}

void FsParticleGenerator::Create(
    class FsParticleStore *particleStore,
    FSPARTICLEGENERATORTYPE genType,const YsVec3 &pos,const YsVec3 &baseDir,const double &timeRemain,const double &elevation)
{
	this->particleStore=particleStore;
	this->genType=genType;
	this->pos=pos;
	this->baseDir=baseDir;
	this->timeRemain=timeRemain;
	this->timePassed=0.0;
	this->timeSinceLastGeneration=0.0;
	this->elevation=elevation;
	this->iniSize=3.0;
	this->maxSize=3.0;
	this->growthRate=0.0;

	drag=0.3;
	timeInterval=0.05;
	unitNum=2;
	particleLife=5.0;
	this->baseDir.Normalize();
}

void FsParticleGenerator::SetTimeInterval(const double &timeInterval)
{
	this->timeInterval=timeInterval;
}

void FsParticleGenerator::SetParticleLife(const double &particleLife)
{
	this->particleLife=particleLife;
}
void FsParticleGenerator::SetSize(double iniSize,double maxSize,double growthRate)
{
	this->iniSize=iniSize;
	this->maxSize=maxSize;
	this->growthRate=growthRate;
}

void FsParticleGenerator::SetUnitNum(int n)
{
	this->unitNum=n;
}

void FsParticleGenerator::Move(const double &dt)
{
	if(timeRemain>=0)
	{
		timeRemain-=dt;
		timePassed+=dt;
		timeSinceLastGeneration+=dt;
		if(timeSinceLastGeneration>timeInterval)
		{
			timeSinceLastGeneration-=timeInterval;

			if(baseDir!=YsOrigin())
			{
				YsVec3 u,v;
				u=baseDir.GetArbitraryPerpendicularVector();
				u.Normalize();
				v=baseDir^u;
				int i;
				for(i=0; i<unitNum; i++)
				{
					double s,t,m;
					s=(double)(rand()%1000-500)/1000.0;
					t=(double)(rand()%1000-500)/1000.0;
					m=(double)(rand()%1000)/50.0;

					YsVec3 vel=(baseDir+u*s+v*t)*m;
					FsParticle *particle=particleStore->CreateParticle(pos,vel,drag,particleLife,elevation);
					particle->SetSize(iniSize,maxSize,growthRate);
					ApplyParticleType(particle);
				}
			}
			else
			{
				int i;
				for(i=0; i<unitNum; i++)
				{
					double vx,vy,vz;

					vx=(double)(rand()%1000)/50.0;
					vy=(double)(rand()%1000)/50.0;
					vz=(double)(rand()%1000)/50.0;

					YsVec3 vel(vx,vy,vz);
					FsParticle *particle=particleStore->CreateParticle(pos,vel,drag,particleLife,elevation);
					ApplyParticleType(particle);
				}
			}
		}
	}
}

YSBOOL FsParticleGenerator::IsAlive(void) const
{
	if(timeRemain>0.0)
	{
		return YSTRUE;
	}
	return YSFALSE;
}

void FsParticleGenerator::ApplyParticleType(FsParticle *particle)
{
	switch(genType)
	{
	case FSPARTICLEGENERATOR_BURN:
		particle->SetGravity(YsVec3(0.0,3.0,0.0));
		particle->SetColorTransition(FSPARTICLECOLOR_BURN);
		break;
	}
}

////////////////////////////////////////////////////////////



FsParticleStore::FsParticleStore() : 
    particleList(particleAllocator),availableList(particleAllocator),
    generatorList(generatorAllocator),availableGeneratorList(generatorAllocator)
{
}

FsParticleStore::~FsParticleStore()
{
	Initialize();
}

void FsParticleStore::Initialize(void)
{
	particleList.CleanUp();
	availableList.CleanUp();
	particleAllocator.CollectGarbage();

	generatorList.CleanUp();
	availableGeneratorList.CleanUp();
	generatorAllocator.CollectGarbage();
}

void FsParticleStore::Clear(void)
{
	YsListItem <FsParticle> *ptr;
	while(NULL!=(ptr=particleList.FindNext(NULL)))
	{
		particleList.Transfer(ptr,availableList);
	}

	YsListItem <FsParticleGenerator> *generator;
	while(NULL!=(generator=generatorList.FindNext(NULL)))
	{
		generatorList.Transfer(generator,availableGeneratorList);
	}
}

FsParticle *FsParticleStore::CreateParticle(const YsVec3 &pos,const YsVec3 &vel,const double &drag,const double &timeRemain,const double &elevation)
{
	YsListItem <FsParticle> *newParticle;
	if(availableList.GetN()>0)
	{
		newParticle=availableList.FindNext(NULL);
		availableList.Transfer(newParticle,particleList);
	}
	else
	{
		newParticle=particleList.Create();
	}
	newParticle->dat.Initialize(newParticle);
	newParticle->dat.Create(pos,vel,drag,timeRemain,elevation);
	return &newParticle->dat;
}

FsParticleGenerator *FsParticleStore::CreateGenerator(FSPARTICLEGENERATORTYPE genType,const YsVec3 &pos,const YsVec3 &baseDir,const double &timeRemain,const double &elevation)
{
	YsListItem <FsParticleGenerator> *newGenerator;
	if(availableGeneratorList.GetN()>0)
	{
		newGenerator=availableGeneratorList.FindNext(NULL);
		availableGeneratorList.Transfer(newGenerator,generatorList);
	}
	else
	{
		newGenerator=generatorList.Create();
	}
	newGenerator->dat.Initialize(newGenerator);
	newGenerator->dat.Create(this,genType,pos,baseDir,timeRemain,elevation);
	return &newGenerator->dat;
}

void FsParticleStore::Move(const double &dt,const YsVec3 &wind)
{
	YsListItem <FsParticle> *ptr;
	particleList.RewindPointer();
	while(NULL!=(ptr=particleList.StepPointer()))
	{
		ptr->dat.Move(dt,wind);
		if(ptr->dat.IsAlive()!=YSTRUE)
		{
			particleList.Transfer(ptr,availableList);
		}
	}

	YsListItem <FsParticleGenerator> *generator;
	generatorList.RewindPointer();
	while(NULL!=(generator=generatorList.StepPointer()))
	{
		generator->dat.Move(dt);
		if(generator->dat.IsAlive()!=YSTRUE)
		{
			generatorList.Transfer(generator,availableGeneratorList);
		}
	}
}

void FsParticleStore::AddToParticleManager(class YsGLParticleManager &partMan) const
{
	YsListItem <FsParticle> *ptr;
	particleList.RewindPointer();
	while(nullptr!=(ptr=particleList.StepPointer()))
	{
		auto &particle=ptr->dat;
		YsColor col=particle.GetColor();
		const YsVec3 &pos=particle.GetPosition();
		const float scale=(float)particle.GetSize();
		const float s0=(float)particle.GetPattern()*0.125f;
		const float t0=0.0f;

		float a=col.Af();
		a*=(float)particle.GetAlphaScale();
		col.SetAf(a);

		partMan.Add(pos,col,scale,s0,t0);
	}
}

