#include "OSCAudioBase.h"
OSCAudioBase* OSCAudioBase::first_route = NULL;

static void dbgPrt(OSCMessage& msg, int addressOffset)
{
	char prt[50];
	msg.getAddress(prt,addressOffset);

	Serial.println(addressOffset);
	Serial.println(prt);
	Serial.println(msg.size());
	Serial.println(); 
}

#if defined(SAFE_RELEASE) // only defined in Dynamic Audio Objects library
//=======================================================================================================
//============================== Dynamic Audio Objects ==================================================
/**
 *	Route message for the creation engine to the correct function.
 */
void OSCAudioBase::routeDynamic(OSCMessage& msg, int addressOffset)
{
    
    if (isStaticTarget(msg,addressOffset,"/ren*","ss")) {renameObject(msg,addressOffset);} 
    #if defined(SAFE_RELEASE) // only defined in Dynamic Audio Objects library
    else if (isStaticTarget(msg,addressOffset,"/cr*O*","ss")) {createObject(msg,addressOffset);}
    else if (isStaticTarget(msg,addressOffset,"/cr*C","s")) {createConnection(msg,addressOffset);} 
    else if (isStaticTarget(msg,addressOffset,"/d*","s")) {destroyObject(msg,addressOffset);}
    #endif // defined(SAFE_RELEASE)
}


/**
 *	Destroy an [OSC]AudioStream or Connection object.
 */
void OSCAudioBase::destroyObject(OSCMessage& msg, int addressOffset)
{
	char buf[50];
	OSCAudioBase* pVictim;
	msg.getString(0,buf,50);
	
	Serial.print("destroyObject: ");
	Serial.println(buf);
	dbgPrt(msg,addressOffset);
		
	pVictim = OSCAudioBase::find(buf);
	Serial.printf("Victim is at: 0x%08X\n",(uint32_t) pVictim);
	Serial.flush();
	if (NULL != pVictim)
		delete pVictim;
	else
		Serial.println("not found!");
}

//============================== OSCAudioStream =========================================================
/**
 *	Create a new [OSC]AudioStream object.
 */
void OSCAudioBase::createObject(OSCMessage& msg, int addressOffset)
{
	char* name;
    char* typ;
    
	void* pNewObj = NULL;
    name = (char*) malloc(50);
    char* nameOrigin = name; // so that free can be used later (as the trim function changes the start address)
    typ = (char*) malloc(50);

	msg.getString(0,typ,50);
	msg.getString(1,name,50);
	
	//Serial.printf("createObject(%s,%s)\n",typ,name);
	//dbgPrt(msg,addressOffset);
	name = trim(name);
    replaceWhiteSpace(name, '_');
    Serial.printf("createObject(%s,%s)\n",typ,name);

    if (strlen(name) == 0) Serial.println("empty name"); // don't allow any kind of "empty" name
	else if (NULL != find(name)) Serial.println("duplicate"); // don't allow duplicate name
#define OSC_CLASS(a,o) else if (0 == strcmp(#a,typ)) pNewObj = new o(name);
	OSC_AUDIO_CLASSES // massive inefficient macro expansion to create object of required type
#undef OSC_CLASS
	
	if (NULL != pNewObj)
	{
		Serial.printf("Created %s as a new %s at %08X\n",name, typ, (uint32_t) pNewObj);
	}
    //address:
    //Serial.printf("Address free(name) in memory: %p\n", &&address);
    free(nameOrigin);
    free(typ);
}

//============================== OSCAudioConnection =====================================================
/**
 *	Create a new [OSC]AudioConnection object.
 */
void OSCAudioBase::createConnection(OSCMessage& msg, int addressOffset)
{
	char* name;
    name = (char*) malloc(50);
	char* nameOrigin = name; // so that free can be used later (as the trim function changes the start address)

	//dbgPrt(msg,addressOffset);
	msg.getString(0,name,50);
    name = trim(name);
    replaceWhiteSpace(name, '_');
    Serial.printf("createConnection(%s)\n",name);
    if (strlen(name) == 0) Serial.println("empty name"); // don't allow any kind of "empty" name
    else {
        OSCAudioConnection* pNewConn = new OSCAudioConnection(name);
        (void) pNewConn;
        Serial.printf("Created at: 0x%08X\n",(uint32_t) pNewConn);
    }
    free(nameOrigin);
}

/**
 *	Rename an [OSC]AudioStream or Connection object.
 *  This could be an /audio function, but that would pollute the 
 *	audio functions' name spaces, so we make it a /dynamic
 *  function instead.
 */
void OSCAudioBase::renameObject(OSCMessage& msg, int addressOffset)
{
	char oldName[50];
    char *newName;
    newName = (char*) malloc(50);
	char* newNameOrigin = newName; // so that free can be used later (as the trim function changes the start address)

	OSCAudioBase* pVictim;
	
	msg.getString(1,newName,50);
    
    newName = trim(newName);
    replaceWhiteSpace(newName, '_');

    if (strlen(newName) == 0) Serial.println("empty name"); // don't allow any kind of "empty" name
	else
    {
        pVictim = OSCAudioBase::find(newName);
        if (NULL == pVictim) // we're not duplicating the name of another object: good
        {
            msg.getString(0,oldName,50);
            
            pVictim = OSCAudioBase::find(oldName);
            if (NULL != pVictim)
            {
                pVictim->setName(newName);
                Serial.printf("renamed(%s) to (%s)\n",oldName,newName);
            }
        }
    }
    free(newNameOrigin);
}


/**
 *	Connect an [OSC]AudioConnection object to specific object ports
 */
void OSCAudioConnection::OSCconnect(OSCMessage& msg, 
								 int addressOffset, 
								 bool zeroToZero) //!< true to use port 0 on both, otherwise they're in the message
{
	char srcn[50],dstn[50];
	AudioStream* src,*dst;
	int srcp=0,dstp=0;
	OSCAudioBase* tmp;
	
	Serial.println("makeConnection");
	dbgPrt(msg,addressOffset);
	
	msg.getString(0,srcn,50);
	if (!zeroToZero)
	{
		srcp = msg.getInt(1);
		msg.getString(2,dstn,50);
		dstp = msg.getInt(3);
	}
	else
		msg.getString(1,dstn,50);
	
	// Find the named OSCAudioBase objects and convert to
	// the corresponding AudioStream: is there a better way?
	tmp = find(srcn); src = (NULL != tmp)?tmp->sibling:NULL;
	tmp = find(dstn); dst = (NULL != tmp)?tmp->sibling:NULL;

	Serial.printf("%s:%d -> %s:%d\n",srcn,srcp,dstn,dstp);
	
	if (NULL != src && NULL != dst)
		connect(*src,(int) srcp,*dst,(int) dstp);
}
#endif // defined(SAFE_RELEASE)


