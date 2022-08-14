#ifndef FSPARTICLE_IS_INCLUDED
#define FSPARTICLE_IS_INCLUDED
/* { */

enum FSPARTICLECOLORTRANSITION
{
	FSPARTICLECOLOR_STATIC,
	FSPARTICLECOLOR_BURN
};

class FsParticle
{
protected:
	YSBOOL exist;
	YsVec3 pos;
	YsVec3 vel;
	double drag;
	double elevation;
	double curSize,maxSize,growthRate;
	double timePassed,timeRemain;
	double fadeTime;
	int pattern;  // 0-7.  Corresponds to texture-atlas parameter s=pattern*0.125.

	YsVec3 gravity;  // Can specify negative gravity for billowing smoke
	YsColor col;
	FSPARTICLECOLORTRANSITION colTrans;

	YsListItem <FsParticle> *thisInTheList;

public:
	void Initialize(YsListItem <FsParticle> *ptr);
	void Create(const YsVec3 &pos,const YsVec3 &vel,const double &drag,const double &timeRemain,const double &elevation);
	void SetGravity(const YsVec3 &gravity);
	void SetColor(const YsColor &col);
	void SetColorTransition(FSPARTICLECOLORTRANSITION colTrans);
	int GetPattern(void) const;

	void SetSize(double curSize,double maxSize,double growthRate);

	/*! This will set current and max sizes the same, and zero growthRate.
	*/
	void SetSize(double curSize);

	/*! Returns the current size.
	*/
	double GetSize(void) const;

	/*! Returns the alpha scale based on the remaining time and fade time.
	*/
	double GetAlphaScale(void) const;

	void Move(const double &dt,const YsVec3 &wind);
	const double TimeRemain(void) const;

	YSBOOL IsAlive(void) const;
	const YsColor &GetColor(void) const;
	const YsVec3 &GetPosition(void) const;
};

enum FSPARTICLEGENERATORTYPE
{
	FSPARTICLEGENERATOR_BURN
};

class FsParticleGenerator
{
protected:
	YsVec3 pos;
	YsVec3 baseDir;
	FSPARTICLEGENERATORTYPE genType;
	double timeRemain,timePassed,timeSinceLastGeneration;
	double iniSize,maxSize,growthRate;
	double elevation;
	class FsParticleStore *particleStore;

	double timeInterval,particleLife,drag;
	int unitNum;

	YsListItem <FsParticleGenerator> *thisInTheList;
public:
	void Initialize(YsListItem <FsParticleGenerator> *ptr);
	void Create(
	    class FsParticleStore *particleStore,
	    FSPARTICLEGENERATORTYPE genType,const YsVec3 &pos,const YsVec3 &baseDir,const double &timeRemain,const double &elevation);
	void SetTimeInterval(const double &timeInterval);
	void SetParticleLife(const double &particleLife);
	void SetUnitNum(int n);
	void SetSize(double iniSize,double maxSize,double growthRate);

	void Move(const double &dt);

	YSBOOL IsAlive(void) const;
protected:
	void ApplyParticleType(FsParticle *particle);
};

class FsParticleStore
{
protected:
	YsListAllocator <FsParticle> particleAllocator;
	YsListContainer <FsParticle> particleList,availableList;

	YsListAllocator <FsParticleGenerator> generatorAllocator;
	YsListContainer <FsParticleGenerator> generatorList,availableGeneratorList;

public:
	FsParticleStore();
	~FsParticleStore();

	void Initialize(void);
	void Clear(void);
	FsParticle *CreateParticle(const YsVec3 &pos,const YsVec3 &vel,const double &drag,const double &timeRemain,const double &elevation);
	FsParticleGenerator *CreateGenerator(FSPARTICLEGENERATORTYPE genType,const YsVec3 &pos,const YsVec3 &baseDir,const double &timeRemain,const double &elevation);

	void Move(const double &dt,const YsVec3 &wind);
	void Draw(const class YsGLParticleManager &partMan) const;

	void AddToParticleManager(class YsGLParticleManager &partMan) const;
};

/* } */
#endif
