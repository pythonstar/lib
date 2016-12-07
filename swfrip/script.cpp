#include "swf.h"
#include "pool.h"
#include "stack.h"

#define STRUCT_IF			0x01
#define STRUCT_ELSE			0x02
#define STRUCT_WHILE		0x03
#define STRUCT_IFFRAME		0x04
#define STRUCT_WITH			0x05
#define STRUCT_FUNCPUSH		0x06
#define STRUCT_FUNCPERSIST	0x07
//#define MAX_STRUCT 4096

struct STRUCT
{
	UI8 ubType;
	UI32 ulStart;
	UI32 ulEnd;
	
	//char *szCode;
};

UI32 CSWF::WriteActions(ACTIONARRAY *lpaaActionArray, UI8 ubLevel, FILE *pfileScript)
{
	char szAction[MAX_STRING], szString[MAX_STRING], szString2[MAX_STRING];
	char szFunction[4096], szOperator[16], szIndent[256], *szParam;
	UI8 fInRegister, fCatch, fFinally;
	UI16 usCatchSize, usFinallySize, usTrySize;
	UI16 iString;
	UI32 ulParam, ulParam2;
	SI32 slParam;
	double sdFloat;
	CBitstream bsActionStream;

	CStack csStack;
	STACK_ENTRY seStackEntry, seStackEntry2, seStackEntry3;

	UI8 ubActionCode, ubPrevAction=0, fInTellTarget=false, fDebug=false;
	UI8 fStartElse=false, fStartWhile=false;
	UI16 usLength=0;
	UI32 ulAction=0, ulProgramCounter=0, ulCurOffset=0;
	
	ACTIONRECORD **lprgarpActions=lpaaActionArray->lprgarpActions;
	UI32 ulNumActions=lpaaActionArray->ulNumActions;
	UI32 ulTotalLength=lpaaActionArray->ulTotalLength;

	STRUCT lprgsStructs[256];
	memset(lprgsStructs,0,256*sizeof(STRUCT));

	bsActionStream.Create(ulTotalLength);

	for(UI32 iAction=0; iAction<ulNumActions; iAction++) 
	{
		bsActionStream.WriteUI8(lprgarpActions[iAction]->ubActionCode);
		if(lprgarpActions[iAction]->usLength)
		{
			bsActionStream.WriteUI16(lprgarpActions[iAction]->usLength);
			bsActionStream.WriteBytes(lprgarpActions[iAction]->lpvActionData,
									  lprgarpActions[iAction]->usLength);
		}

	}

	bsActionStream.Rewind();

	while(ubActionCode=bsActionStream.ReadUI8())
	{
		ulProgramCounter++;

		if(ulProgramCounter>=ulTotalLength) break;
		
		else if(ubActionCode & 0x80 && ubActionCode!=ACTIONCODE_CALL) 
		{
			usLength=bsActionStream.ReadUI16();
			ulProgramCounter+=usLength+2;
		}

		else usLength=0;


		GetActionName(ubActionCode,szAction);
		strcpy(szString,"");
		if(fDebug) fprintf(pfileScript,"/*%s*/\n",szAction);

		/*make indent*/
		memset(szIndent,'\t',ubLevel);
		szIndent[ubLevel]='\0';
		
		if(usLength)
		{
			switch(ubActionCode)
			{
				case ACTIONCODE_IF:
					csStack.Pop(&seStackEntry);
					Negate(seStackEntry.szString);
					RemoveParens(seStackEntry.szString);
						
					slParam=bsActionStream.ReadSI16();
						
					sprintf(szString,"\n%sif(%s)\n%s{\n",	
							szIndent,
							seStackEntry.szString,
							szIndent);

					ubLevel++;
					lprgsStructs[ubLevel].ubType=STRUCT_IF;
					lprgsStructs[ubLevel].ulStart=ulProgramCounter;
					lprgsStructs[ubLevel].ulEnd=ulProgramCounter+slParam;
					//lprgsStructs[ubLevel].szCode=new char[MAX_STRUCT];
					//strcpy(lprgsStructs[ubLevel].szCode,"");
					break;

				case ACTIONCODE_JUMP:
					slParam=bsActionStream.ReadSI16();

					if( lprgsStructs[ubLevel].ubType==STRUCT_IF &&
						ulProgramCounter==lprgsStructs[ubLevel].ulEnd)
					{
						if(slParam>0) fStartElse=true;
						else lprgsStructs[ubLevel].ubType=STRUCT_WHILE;
					}

					else
					{
						//ulProgramCounter+=slParam;
					}

					break;

				case ACTIONCODE_WAITFORFRAME:
				case ACTIONCODE_WAITFORFRAME2:
					/*frame*/
					if(ubActionCode==ACTIONCODE_WAITFORFRAME)
						ulParam=bsActionStream.ReadUI16();
						
					if(ubActionCode==ACTIONCODE_WAITFORFRAME2)
					{
						csStack.Pop(&seStackEntry);
						ulParam=atoi(seStackEntry.szString);
						
					}

					/*skipcount*/
					ulParam2=bsActionStream.ReadUI8();
					
					sprintf(szString,"ifFrameLoaded(%lu)\n%s{\n",ulParam+1,szIndent);
					
					/*actions*/
					
					ubLevel++;
					lprgsStructs[ubLevel].ubType=STRUCT_IFFRAME;
					lprgsStructs[ubLevel].ulStart=ulAction;
					lprgsStructs[ubLevel].ulEnd=ulAction+ulParam2;
					//lprgsStructs[ubLevel].szCode=new char[MAX_STRUCT];
					//strcpy(lprgsStructs[ubLevel].szCode,"");
					break;

				case ACTIONCODE_DEFINEFUNCTION:
					/*function name*/
					bsActionStream.ReadString(szParam);
					sprintf(szString,"\n%sfunction %s(",szIndent,szParam);
						
					ubLevel++;
					if(!szParam[0])
					{
						lprgsStructs[ubLevel].ubType=STRUCT_FUNCPUSH;
						strcpy(seStackEntry.szString,"<function>");
						seStackEntry.ubType=STACK_STRING;
						csStack.Push(&seStackEntry);
					}
					else lprgsStructs[ubLevel].ubType=STRUCT_FUNCPERSIST;

					delete szParam;
						
					/*parameters*/
					ulParam=bsActionStream.ReadUI16();
						
					for(iString=0; iString<ulParam; iString++)
					{
						bsActionStream.ReadString(szParam);
						if(iString) strcat(szString,",");
						strcat(szString,szParam);
						delete szParam;
					}

					sprintf(szString2,")\n%s{\n",szIndent);
					strcat(szString,szString2);

					/*code*/
					ulParam=bsActionStream.ReadUI16();
						
					lprgsStructs[ubLevel].ulStart=ulProgramCounter;
					lprgsStructs[ubLevel].ulEnd=ulProgramCounter+ulParam;
					//lprgsStructs[ubLevel].szCode=new char[MAX_STRUCT];
					//strcpy(lprgsStructs[ubLevel].szCode,"");
					break;

				case ACTIONCODE_DEFINEFUNCTION2:
					/*function name*/
					bsActionStream.ReadString(szParam);
					sprintf(szString,"function2 %s(",szParam);

					/*parameters*/
					ulParam=bsActionStream.ReadUI16();
												
					/*registers*/
					ulParam2=bsActionStream.ReadUI8();
						
					/*PreloadParentFlag*/
					ulParam2=bsActionStream.ReadUB(1);
					if(ulParam2==0) strcat(szString,"false");
					if(ulParam2==1) strcat(szString,"true");

					/*PreloadRootFlag*/
					ulParam2=bsActionStream.ReadUB(1);
					if(ulParam2==0) strcat(szString,",false");
					if(ulParam2==1) strcat(szString,",true");

					/*SuppressSuperFlag*/
					ulParam2=bsActionStream.ReadUB(1);
					if(ulParam2==0) strcat(szString,",false");
					if(ulParam2==1) strcat(szString,",true");

					/*PreloadSuperFlag*/
					ulParam2=bsActionStream.ReadUB(1);
					if(ulParam2==0) strcat(szString,",false");
					if(ulParam2==1) strcat(szString,",true");

					/*SuppressArgumentsFlag*/
					ulParam2=bsActionStream.ReadUB(1);
					if(ulParam2==0) strcat(szString,",false");
					if(ulParam2==1) strcat(szString,",true");

					/*PreloadArgumentsFlag*/
					ulParam2=bsActionStream.ReadUB(1);
					if(ulParam2==0) strcat(szString,",false");
					if(ulParam2==1) strcat(szString,",true");

					/*SuppressThisFlag*/
					ulParam2=bsActionStream.ReadUB(1);
					if(ulParam2==0) strcat(szString,",false");
					if(ulParam2==1) strcat(szString,",true");

					/*PreloadThisFlag*/
					ulParam2=bsActionStream.ReadUB(1);
					if(ulParam2==0) strcat(szString,",false");
					if(ulParam2==1) strcat(szString,",true");

					/*reserved*/
					ulParam2=bsActionStream.ReadUB(7);

					/*PreloadGlobalFlag*/
					ulParam2=bsActionStream.ReadUB(1);
					if(ulParam2==0) strcat(szString,",false");
					if(ulParam2==1) strcat(szString,",true");
						
					/*parameters*/
					for(iString=1; iString<ulParam; iString++)
					{
						/*register*/
						ulParam2=bsActionStream.ReadUB(7);
						sprintf(szString2,",%lu",ulParam2);
						strcat(szString,szString2);

						/*parameter name*/
						bsActionStream.ReadString(szParam);
						sprintf(szString2,",%s",szParam);
						strcat(szString,szString2);
					}

					sprintf(szString2,")\n%s{\n",szIndent);
					strcat(szString,szString2);
						
					/*code*/
					ulParam=bsActionStream.ReadUI16();
					strcat(szString,"\n}\n\n");

					break;

				case ACTIONCODE_TRY:
					/*reserved*/
					bsActionStream.ReadUB(5);
						
					/*flags*/
					fInRegister=bsActionStream.ReadUB(1);
					fFinally=bsActionStream.ReadUB(1);
					fCatch=bsActionStream.ReadUB(1);
						
					/*sizes*/
					usTrySize=bsActionStream.ReadUI16();
					usCatchSize=bsActionStream.ReadUI16();
					usFinallySize=bsActionStream.ReadUI16();

					/*CatchName*/
					if(fInRegister)
					{
						bsActionStream.ReadString(szParam);
						sprintf(szString2,"\"%s\"",szParam);
						strcat(szString,szString2);

						ulParam=bsActionStream.ReadUI8();
						sprintf(szString2,",%lu",ulParam);
						strcat(szString,szString2);
					}

						
					if(fCatch)
					{
					}

					if(fFinally)
					{
					}
					
 					break;

				case ACTIONCODE_WITH:
					ulParam=bsActionStream.ReadUI16();
					//bsActionStream.ReadString(szParam);
					csStack.Pop(&seStackEntry);
					sprintf(szString,"\n%swith(\"%s\")\n%s{\n",
							szIndent,seStackEntry.szString,szIndent);
					//delete szParam;
					ubLevel++;
					lprgsStructs[ubLevel].ubType=STRUCT_WITH;
					lprgsStructs[ubLevel].ulStart=ulProgramCounter;
					lprgsStructs[ubLevel].ulEnd=ulProgramCounter+ulParam;
					//lprgsStructs[ubLevel].szCode=new char[MAX_STRUCT];
					//strcpy(lprgsStructs[ubLevel].szCode,"");
					break;

				case ACTIONCODE_GOTOFRAME:
					ulParam=bsActionStream.ReadUI16();

					/*check if next action is play*/
					ulParam2=bsActionStream.GetUI8();

					if(ulParam2==ACTIONCODE_PLAY)
						sprintf(szString,"gotoAndPlay(%lu);\n",ulParam+1);

					else sprintf(szString,"gotoAndStop(%lu);\n",ulParam+1);

					break;

				case ACTIONCODE_GETURL:
					/*url*/
					bsActionStream.ReadString(szParam);
					sprintf(szString,"getURL(\"%s\"",szParam);
					delete szParam;
										
					/*target*/
					bsActionStream.ReadString(szParam);
					
					if(szParam[0]) 
					{
						sprintf(szString2,",\"%s\"",szParam);
						strcat(szString,szString2);
					}
					delete szParam;

					strcat(szString,");\n");
					break;

				case ACTIONCODE_GETURL2:
						/*method*/
						ulParam=bsActionStream.ReadUB(2);

						sprintf(szString,"getURL(");
						
						switch(ulParam)
						{
							case 0: strcat(szString2,"NONE"); break;
							case 1: strcat(szString2,"GET"); break;
							case 2: strcat(szString2,"POST"); break;
							default: strcat(szString2,"invalid"); break;
						}

						/*reserved*/
						ulParam=bsActionStream.ReadUB(4);
						
						/*destination*/
						ulParam=bsActionStream.ReadUB(1);
						if(ulParam==0) strcat(szString2,",BROWSER");
						if(ulParam==1) strcat(szString2,",SPRITE");
						
						/*load variables*/
						ulParam=bsActionStream.ReadUB(1);
						if(ulParam==0) strcat(szString2,",false");
						if(ulParam==1) strcat(szString2,",true");

						csStack.Pop(&seStackEntry);
						//csStack.ConvertToString(&seStackEntry);

						csStack.Pop(&seStackEntry2);
						//csStack.ConvertToString(&seStackEntry2);

						strcat(szString,seStackEntry2.szString);

						if(seStackEntry.szString[0])
						{
							strcat(szString,",");
							strcat(szString,seStackEntry2.szString);
						}

						strcat(szString,");\n");

						break;

				case ACTIONCODE_SETTARGET:
					bsActionStream.ReadString(szParam);
											
					if(fInTellTarget)
					{
						szIndent[strlen(szIndent)-1]='\0';
						fprintf(pfileScript,"%s}\n\n",szIndent);
						//sprintf(szString,"}\n");
						ubLevel--;
						fInTellTarget=false;
					}

					if(szParam[0])
					{
						sprintf(szString,"tellTarget(\"%s\")\n%s{\n",szParam,szIndent);
						ubLevel++;
						fInTellTarget=true;
					}

					delete szParam;
					break;

				case ACTIONCODE_GOTOLABEL:
					bsActionStream.ReadString(szParam);

					/*check if next action is play*/
					ulParam2=bsActionStream.GetUI8();

					if(ulParam2==ACTIONCODE_PLAY)
						sprintf(szString,"gotoAndPlay(\"%s\");\n",szParam);

					else sprintf(szString,"gotoAndStop(\"%s\");\n",szParam);

					delete szParam;
					break;

				case ACTIONCODE_PUSH:
					while(bsActionStream.GetByteOffset()<ulProgramCounter)
					{
						seStackEntry.ubType=bsActionStream.ReadUI8();
						
						switch(seStackEntry.ubType)
						{
							case STACK_STRING:
								bsActionStream.ReadString(szParam);
								if(fDebug) fprintf(pfileScript,"\"%s\"",szParam);
								strcpy(seStackEntry.szString,szParam);
								delete(szParam);
								break;	

							case STACK_FLOAT:
								ulParam=bsActionStream.ReadUI32	();
								if(fDebug) fprintf(pfileScript,"%08X",ulParam);
								seStackEntry.ulDoubleHigh=ulParam;
								break;

							case STACK_REG:
								ulParam=bsActionStream.ReadUI8();
								if(fDebug) fprintf(pfileScript,"r%i",ulParam);
								seStackEntry.ubRegister=ulParam;
								break;

							case STACK_BOOL:
								ulParam=bsActionStream.ReadUI8();
								if(fDebug)
								{
									if(ulParam) fprintf(pfileScript,"true");
									else fprintf(pfileScript,"false");
								}
								seStackEntry.ubBoolean=ulParam;
								break;

							case STACK_DOUBLE:
								ulParam=bsActionStream.ReadUI32();
								ulParam2=bsActionStream.ReadUI32();
								seStackEntry.ulDoubleHigh=ulParam;
								seStackEntry.ulDoubleLow=ulParam2;														
								if(fDebug) fprintf(pfileScript,"0x%08X%08X",ulParam,ulParam2);
								break;

							case STACK_INT:
								slParam=bsActionStream.ReadSI32();
								if(fDebug) fprintf(pfileScript,"%i",slParam);
								seStackEntry.slInteger=slParam;
								break;

							case STACK_CONSTANT:
								ulParam=bsActionStream.ReadUI8();
								if(fDebug) fprintf(pfileScript,"\"%s\"",csStack.cpoolConstantPool.Get(ulParam));
								seStackEntry.usConstant=ulParam;
								break;

							case 9: /*constant 16*/
								seStackEntry.ubType=STACK_CONSTANT;
								ulParam=bsActionStream.ReadUI16();
								if(fDebug) fprintf(pfileScript,"\"%s\"",csStack.cpoolConstantPool.Get(ulParam));
								seStackEntry.usConstant=ulParam;
								break;

							default:
								if(fDebug) fprintf(pfileScript,"UNKNOWN PUSH TYPE: %i",seStackEntry.ubType);
						}
						if(fDebug) fprintf(pfileScript,"\n");
						csStack.Push(&seStackEntry);
					}
					break;

					case ACTIONCODE_GOTOFRAME2:
						sprintf(szString,"gotoAnd");

						/*reserved*/
						bsActionStream.ReadUB(6);
						
						/*SceneBiasFlag*/
						ulParam=bsActionStream.ReadUB(1);
												 
						/*Playflag*/
						ulParam2=bsActionStream.ReadUB(1);
						if(ulParam2==0) strcat(szString,"Stop(");
						if(ulParam2==1) strcat(szString,"Play(");

						/*frame number/label*/
						csStack.Pop(&seStackEntry);
						strcat(szString,seStackEntry.szString);
												
						/*SceneBias*/
						if(ulParam)
						{
							ulParam2=bsActionStream.ReadUI16();
							sprintf(szString2," + %lu",ulParam2);
							strcat(szString,szString2);
						}

						strcat(szString,");\n");

						break;

					case ACTIONCODE_CONSTANTPOOL:
						ulParam=bsActionStream.ReadUI16();
						csStack.cpoolConstantPool.Clear();
						csStack.cpoolConstantPool.SetSize(ulParam);
						
						for(iString=0; iString<ulParam; iString++)
						{
							bsActionStream.ReadString(szParam);
							csStack.cpoolConstantPool.Add(szParam);
							delete szParam;
						}

						break;

					case ACTIONCODE_STOREREGISTER:
						ulParam=bsActionStream.ReadUI8();
						csStack.Pop(&seStackEntry);
						csStack.StoreRegister(ulParam,&seStackEntry);
						csStack.Push(&seStackEntry);
						break;
								
				default:
					sprintf(szString,"//%s\n",szAction);
					bsActionStream.Seek(usLength);
			}
		}

		else
		{
			switch(ubActionCode) /*actions with no data*/
			{
				/*binary operators*/
				case ACTIONCODE_ADD:
				case ACTIONCODE_ADD2:
				case ACTIONCODE_SUBTRACT:
				case ACTIONCODE_MULTIPLY:
				case ACTIONCODE_DIVIDE:
				case ACTIONCODE_EQUALS:
				case ACTIONCODE_EQUALS2:
				case ACTIONCODE_STRICTEQUALS:
				case ACTIONCODE_LESS:
				case ACTIONCODE_LESS2:
				case ACTIONCODE_STRINGLESS:
				case ACTIONCODE_GREATER:
				case ACTIONCODE_STRINGGREATER:
				case ACTIONCODE_AND:
				case ACTIONCODE_OR:
				case ACTIONCODE_BITAND:
				case ACTIONCODE_BITLSHIFT:
				case ACTIONCODE_BITOR:
				case ACTIONCODE_BITRSHIFT:
				case ACTIONCODE_BITURSHIFT:
				case ACTIONCODE_BITXOR:
					switch(ubActionCode)
					{
						case ACTIONCODE_ADD:			strcpy(szOperator,"+"); break;
						case ACTIONCODE_ADD2:			strcpy(szOperator,"+"); break;
						case ACTIONCODE_SUBTRACT:		strcpy(szOperator,"-"); break;
						case ACTIONCODE_MULTIPLY:		strcpy(szOperator,"*"); break;
						case ACTIONCODE_DIVIDE:			strcpy(szOperator,"/"); break;
						case ACTIONCODE_EQUALS:			strcpy(szOperator,"=="); break;
						case ACTIONCODE_EQUALS2:		strcpy(szOperator,"=="); break;
						case ACTIONCODE_STRICTEQUALS:	strcpy(szOperator,"==="); break;
						case ACTIONCODE_LESS:			strcpy(szOperator,"<"); break;
						case ACTIONCODE_LESS2:			strcpy(szOperator,"<"); break;
						case ACTIONCODE_STRINGLESS:		strcpy(szOperator,"<"); break;
						case ACTIONCODE_GREATER:		strcpy(szOperator,">"); break;
						case ACTIONCODE_STRINGGREATER:	strcpy(szOperator,">"); break;
						case ACTIONCODE_AND:			strcpy(szOperator,"&&"); break;
						case ACTIONCODE_OR:				strcpy(szOperator,"||"); break;
						case ACTIONCODE_BITAND:			strcpy(szOperator,"&"); break;
						case ACTIONCODE_BITLSHIFT:		strcpy(szOperator,"<<"); break;
						case ACTIONCODE_BITOR:			strcpy(szOperator,"|"); break;
						case ACTIONCODE_BITRSHIFT:		strcpy(szOperator,">>"); break;
						case ACTIONCODE_BITURSHIFT:		strcpy(szOperator,">>>"); break;
						case ACTIONCODE_BITXOR:			strcpy(szOperator,"^"); break;
					}

					csStack.Pop(&seStackEntry);
					csStack.Pop(&seStackEntry2);

					if( ubActionCode==ACTIONCODE_ADD || 
						ubActionCode==ACTIONCODE_ADD2 ||
						ubActionCode==ACTIONCODE_SUBTRACT)
					{
						RemoveParens(seStackEntry.szString);
						RemoveParens(seStackEntry2.szString);
					}

					if(!strcmp(seStackEntry2.szString,"0"))
					{
						if(!strcmp(szOperator,"+"))
							sprintf(szString,"%s",seStackEntry.szString);
					
						else if(!strcmp(szOperator,"-"))
							sprintf(szString,"(-%s)",seStackEntry.szString);
					}
					
					else sprintf(szString,"(%s %s %s)",seStackEntry2.szString,
								 szOperator,seStackEntry.szString);
					
					strcpy(seStackEntry.szString,szString);
					csStack.Push(&seStackEntry);
					strcpy(szString,"");
					//sprintf(szString,"//%s",seStackEntry.szString);
					break;

				case ACTIONCODE_MODULO: /*x%y*/
					/*x*/
					csStack.Pop(&seStackEntry);
					strcpy(szString2,seStackEntry.szString);

					/*y*/
					csStack.Pop(&seStackEntry);
					strcpy(szString,seStackEntry.szString);
					
					sprintf(seStackEntry.szString,"(%s % %s)",szString2,szString);
					csStack.Push(&seStackEntry);
					strcpy(szString,"");
					//sprintf(szString,"//%s",seStackEntry.szString);
					break;

				case ACTIONCODE_NOT:
					csStack.Pop(&seStackEntry);
					Negate(seStackEntry.szString);
					csStack.Push(&seStackEntry);
					strcpy(szString,"");
					//sprintf(szString,"//%s",seStackEntry.szString);
					break;

				case ACTIONCODE_DECREMENT:
					csStack.Pop(&seStackEntry);
					sprintf(szString,"(%s--)",seStackEntry.szString);
					csStack.Push(&seStackEntry);
					strcpy(szString,"");
					break;

				case ACTIONCODE_INCREMENT:
					csStack.Pop(&seStackEntry);
					sprintf(szString,"(%s++)",seStackEntry.szString);
					csStack.Push(&seStackEntry);
					strcpy(szString,"");
					break;

				case ACTIONCODE_STOPSOUNDS:
					sprintf(szString,"stopAllSounds();\n");
					break;

				case ACTIONCODE_PLAY:
					if(ubPrevAction==ACTIONCODE_GOTOFRAME) break;
					if(ubPrevAction==ACTIONCODE_GOTOLABEL) break;
					sprintf(szString,"play();\n");
					break;

				case ACTIONCODE_STOP:
					if(ubPrevAction==ACTIONCODE_GOTOFRAME) break;
					if(ubPrevAction==ACTIONCODE_GOTOLABEL) break;
					sprintf(szString,"stop();\n");
					break;

				case ACTIONCODE_PUSHDUPLICATE:
					csStack.Pop(&seStackEntry);
					csStack.Push(&seStackEntry);
					csStack.Push(&seStackEntry);
					break;

				case ACTIONCODE_POP:
					csStack.Pop(&seStackEntry);
					RemoveParens(seStackEntry.szString);
					sprintf(szString,"%s;\n",seStackEntry.szString);
					break;

				case ACTIONCODE_STRINGLENGTH:
					csStack.Pop(&seStackEntry);
					sprintf(szString,"strlen(%s)",seStackEntry.szString);
					strcpy(seStackEntry.szString,szString);
					csStack.Push(&seStackEntry);
					strcpy(szString,"");
					break;

				case ACTIONCODE_MBSTRINGLENGTH:
					csStack.Pop(&seStackEntry);
					sprintf(szString,"mbstrlen(%s)",seStackEntry.szString);
					strcpy(seStackEntry.szString,szString);
					csStack.Push(&seStackEntry);
					strcpy(szString,"");
					break;

				case ACTIONCODE_GETVARIABLE:
					csStack.Pop(&seStackEntry);
					csStack.Push(&seStackEntry);
					strcpy(szString,"");
					//sprintf(szString,"//%s",seStackEntry.szString);
					break;

				case ACTIONCODE_SETVARIABLE:
					/*value*/
					csStack.Pop(&seStackEntry);
					RemoveParens(seStackEntry.szString);
												
					/*varible name*/
					csStack.Pop(&seStackEntry2);
										
					sprintf(szString,"%s = %s;\n",seStackEntry2.szString,seStackEntry.szString);			
					break;

				case ACTIONCODE_GETMEMBER: 
					csStack.Pop(&seStackEntry);
					sprintf(szString2,".%s",seStackEntry.szString);
							
					csStack.Pop(&seStackEntry2);
										
					sprintf(szString,"%s.%s",seStackEntry2.szString,seStackEntry.szString);
					strcpy(seStackEntry.szString,szString);
					
					csStack.Push(&seStackEntry);
					strcpy(szString,"");
					//sprintf(szString,"//%s",szString2);
					break;

				case ACTIONCODE_SETMEMBER:
					csStack.Pop(&seStackEntry);
					RemoveParens(seStackEntry.szString);
					sprintf(szString2,"%s",seStackEntry.szString);
							
					csStack.Pop(&seStackEntry);
					csStack.Pop(&seStackEntry2);
										
					sprintf(szString,"%s.%s = %s;\n",seStackEntry2.szString,
							seStackEntry.szString,szString2);
					break;

				case ACTIONCODE_GETPROPERTY:
					csStack.Pop(&seStackEntry);
					csStack.ConvertToString(&seStackEntry,true);
					csStack.Pop(&seStackEntry2);
					RemoveQuotes(seStackEntry2.szString);
															
					if(!seStackEntry2.szString[0])
						sprintf(szString,"getProperty(\"%s\",%s)",
								seStackEntry2.szString,seStackEntry.szString);

					else sprintf(szString,"%s",seStackEntry.szString);
					
					strcpy(seStackEntry.szString,szString);
					csStack.Push(&seStackEntry);
					strcpy(szString,"");
					//sprintf(szString,"//%s",seStackEntry.szString);
					break;

				case ACTIONCODE_SETPROPERTY:
					csStack.Pop(&seStackEntry);
					RemoveParens(seStackEntry.szString);
					csStack.Pop(&seStackEntry2);
					csStack.ConvertToString(&seStackEntry2,true);
					csStack.Pop(&seStackEntry3);
					RemoveQuotes(seStackEntry3.szString);
															
					sprintf(szString,"setProperty(\"%s\",%s,%s);\n",
							seStackEntry3.szString,
							seStackEntry2.szString,
							seStackEntry.szString);
					break;

				case ACTIONCODE_DEFINELOCAL:
					/*value*/
					csStack.Pop(&seStackEntry);
					RemoveParens(seStackEntry.szString);
											
					/*name*/
					csStack.Pop(&seStackEntry2);
														
					sprintf(szString,"var %s = %s;\n",
							seStackEntry2.szString,seStackEntry.szString);
					break;

				case ACTIONCODE_DEFINELOCAL2:
					csStack.Pop(&seStackEntry);
					sprintf(szString,"var %s;\n",seStackEntry.szString);
					break;

				case ACTIONCODE_TOINTEGER:
					csStack.Pop(&seStackEntry);
					sprintf(szString,"int(%s)",seStackEntry.szString);
					strcpy(seStackEntry.szString,szString);
					csStack.Push(&seStackEntry);
					strcpy(szString,"");
					break;

				case ACTIONCODE_CHARTOASCII:
				case ACTIONCODE_MBCHARTOASCII:
					csStack.Pop(&seStackEntry);
					sprintf(szString,"ascii(%s)",seStackEntry.szString);
					strcpy(seStackEntry.szString,szString);
					csStack.Push(&seStackEntry);
					strcpy(szString,"");
					break;

				case ACTIONCODE_ASCIITOCHAR:
				case ACTIONCODE_MBASCIITOCHAR:
					csStack.Pop(&seStackEntry);
					sprintf(szString,"char(%s)",seStackEntry.szString);
					strcpy(seStackEntry.szString,szString);
					csStack.Push(&seStackEntry);
					strcpy(szString,"");
					break;

				case ACTIONCODE_SETTARGET2:
					csStack.Pop(&seStackEntry);
					strcpy(szString2,seStackEntry.szString);

					if(fInTellTarget)
					{
						szIndent[strlen(szIndent)-1]='\0';
						fprintf(pfileScript,"%s}\n\n",szIndent);
						//sprintf(szString,"}\n");
						ubLevel--;
						fInTellTarget=false;
					}

					if(szString2[0])
					{
						sprintf(szString,"tellTarget(\"%s\")\n%s{\n",szString2,szIndent);
						ubLevel++;
						fInTellTarget=true;
					}

					break;

				case ACTIONCODE_CLONESPRITE:
					/*depth*/
					csStack.Pop(&seStackEntry);
					ulParam=atoi(seStackEntry.szString);
					
					/*target*/
					csStack.Pop(&seStackEntry);
										
					/*source*/
					csStack.Pop(&seStackEntry2);
					
					sprintf(szString,"duplicateMovieClip(\"%s\",%s,%lu);\n",
							seStackEntry2.szString,seStackEntry.szString,ulParam);
					break;

				case ACTIONCODE_REMOVESPRITE:
					/*target*/
					csStack.Pop(&seStackEntry);
					sprintf(szString,"removeMovieClip(\"%s\");\n",seStackEntry.szString);
					break;

				case ACTIONCODE_STARTDRAG:
					/*target*/
					csStack.Pop(&seStackEntry);
					
					/*lockcenter*/
					csStack.Pop(&seStackEntry2);
					RemoveQuotes(seStackEntry.szString);
					
					sprintf(szString,"startDrag(\"%s\",%s",
							seStackEntry.szString,seStackEntry2.szString);

					/*constrain*/
					csStack.Pop(&seStackEntry);
					ulParam=atoi(seStackEntry.szString);

					strcpy(szString2,"");

					if(ulParam)
					{
						/*bottom*/
						csStack.Pop(&seStackEntry);
						strcpy(szString2,seStackEntry.szString);

						/*right*/
						csStack.Pop(&seStackEntry);
						strcat(szString2,",");
						strcat(szString2,seStackEntry.szString);
						
						/*top*/
						csStack.Pop(&seStackEntry);
						strcat(szString2,",");
						strcat(szString2,seStackEntry.szString);
						
						/*left*/
						csStack.Pop(&seStackEntry);
						strcat(szString2,",");
						strcat(szString2,seStackEntry.szString);
					}

					strcat(szString,szString2);
					strcat(szString,");\n");
					break;

				case ACTIONCODE_TRACE:
					csStack.Pop(NULL);
					break;

				case ACTIONCODE_GETTIME:
					sprintf(seStackEntry.szString,"getTimer()");
					csStack.Push(&seStackEntry);
					strcpy(szString,"");
					break;

				case ACTIONCODE_DELETE:
					csStack.Pop(&seStackEntry);
					csStack.Pop(&seStackEntry2);
					
					sprintf(szString,"delete %s.%s;\n",
							seStackEntry.szString,seStackEntry2.szString);
					break;

				case ACTIONCODE_DELETE2:
					csStack.Pop(&seStackEntry);
					sprintf(szString,"delete %s;\n",seStackEntry.szString);
					break;

				case ACTIONCODE_NEWOBJECT:
					csStack.Pop(&seStackEntry);
					sprintf(szString,"new %s(",seStackEntry.szString);

					csStack.Pop(&seStackEntry);
					ulParam=atoi(seStackEntry.szString);

					for(ulParam2=0; ulParam2<ulParam; ulParam2++)
					{
						csStack.Pop(&seStackEntry);
						RemoveParens(seStackEntry.szString);
						if(ulParam2) strcat(szString,",");
						strcat(szString,seStackEntry.szString);
					}

					strcat(szString,")");
					strcpy(seStackEntry.szString,szString);
					csStack.Push(&seStackEntry);
					strcpy(szString,"");
					break;

				case ACTIONCODE_TARGETPATH:
					csStack.Pop(&seStackEntry);
					sprintf(szString,"targetPath(\"%s\")",seStackEntry.szString);
					csStack.Push(&seStackEntry);
					strcpy(szString,"");
					break;

				case ACTIONCODE_TONUMBER:
					csStack.Pop(&seStackEntry);
					sprintf(szString,"toNumber(%s)",seStackEntry.szString);
					csStack.Push(&seStackEntry);
					strcpy(szString,"");
					break;

				case ACTIONCODE_TOSTRING:
					csStack.Pop(&seStackEntry);
					sprintf(szString,"toString(%s)",seStackEntry.szString);
					csStack.Push(&seStackEntry);
					strcpy(szString,"");
					break;

				case ACTIONCODE_TYPEOF:
					csStack.Pop(&seStackEntry);
					sprintf(szString,"typeof(%s)",seStackEntry.szString);
					strcpy(seStackEntry.szString,szString);
					csStack.Push(&seStackEntry);
					strcpy(szString,"");
					break;

				case ACTIONCODE_RETURN:
					csStack.Pop(&seStackEntry);
					break;

				case ACTIONCODE_STACKSWAP:
					csStack.Pop(&seStackEntry);
					csStack.Pop(&seStackEntry2);
					csStack.Push(&seStackEntry);
					csStack.Push(&seStackEntry2);
					break;

				case ACTIONCODE_INSTANCEOF:
					csStack.Pop(&seStackEntry);
					csStack.Pop(&seStackEntry2);
					sprintf(szString,"instanceof(%s,%s)",seStackEntry.szString,seStackEntry.szString);
					strcpy(seStackEntry.szString,szString);
					csStack.Push(&seStackEntry);
					strcpy(szString,"");
					break;

				case ACTIONCODE_RANDOMNUMBER:
					csStack.Pop(&seStackEntry);
					sprintf(szString,"random(%s)",seStackEntry.szString);
					strcpy(seStackEntry.szString,szString);
					csStack.Push(&seStackEntry);
					strcpy(szString,"");
					break;

				/*this tag has no data, mistakenly given code>0x80*/
				case ACTIONCODE_CALL:
					csStack.Pop(&seStackEntry);
					sprintf(szString,"call(\"%s\");\n",seStackEntry.szString);
					//csStack.Push(&seStackEntry);
					break;

				case ACTIONCODE_CALLFUNCTION:
					csStack.Pop(&seStackEntry);
					sprintf(szString,"%s(",seStackEntry.szString);

					csStack.Pop(&seStackEntry);
					ulParam=atoi(seStackEntry.szString);

					for(ulParam2=0; ulParam2<ulParam; ulParam2++)
					{
						csStack.Pop(&seStackEntry);
						RemoveParens(seStackEntry.szString);
						if(ulParam2) strcat(szString,",");
						strcat(szString,seStackEntry.szString);
					}

					strcat(szString,")");
					strcpy(seStackEntry.szString,szString);
					csStack.Push(&seStackEntry);
					strcpy(szString,"");
					break;

				case ACTIONCODE_CALLMETHOD:
					csStack.Pop(&seStackEntry);
					csStack.Pop(&seStackEntry2);

					sprintf(szString,"%s.%s(",
						seStackEntry2.szString,seStackEntry.szString);

					csStack.Pop(&seStackEntry);
					ulParam=atoi(seStackEntry.szString);

					for(ulParam2=0; ulParam2<ulParam; ulParam2++)
					{
						csStack.Pop(&seStackEntry);
						RemoveParens(seStackEntry.szString);
						if(ulParam2) strcat(szString,",");
						strcat(szString,seStackEntry.szString);
					}

					strcat(szString,")");
					strcpy(seStackEntry.szString,szString);
					csStack.Push(&seStackEntry);
					strcpy(szString,"");
					break;

				case ACTIONCODE_NEWMETHOD:
					csStack.Pop(&seStackEntry);
					csStack.Pop(&seStackEntry2);

					sprintf(szString,"%s.%s(",
						seStackEntry2.szString,seStackEntry.szString);

					csStack.Pop(&seStackEntry);
					ulParam=atoi(seStackEntry.szString);

					for(ulParam2=0; ulParam2<ulParam; ulParam2++)
					{
						csStack.Pop(&seStackEntry);
						RemoveParens(seStackEntry.szString);
						if(ulParam2) strcat(szString,",");
						strcat(szString,seStackEntry.szString);
					}

					strcat(szString,")");
					strcpy(seStackEntry.szString,szString);
					csStack.Push(&seStackEntry);
					strcpy(szString,"");
					break;

				case ACTIONCODE_STRINGADD:
					csStack.Pop(&seStackEntry);
					csStack.Pop(&seStackEntry2);
					sprintf(seStackEntry3.szString,"%s + %s",
							seStackEntry2.szString,
							seStackEntry.szString);

					csStack.Push(&seStackEntry3);
					break;

				case ACTIONCODE_STRINGEXTRACT:
					csStack.Pop(&seStackEntry);
					RemoveParens(seStackEntry.szString);
					csStack.Pop(&seStackEntry2);
					RemoveParens(seStackEntry.szString);
					csStack.Pop(&seStackEntry3);

					sprintf(szString,"substring(%s,%s,%s)",
							seStackEntry3.szString,
							seStackEntry2.szString,
							seStackEntry.szString);

					strcpy(seStackEntry.szString,szString);
					csStack.Push(&seStackEntry);
					strcpy(szString,"");
					break;

				case ACTIONCODE_INITARRAY:
					/*#elements*/
					csStack.Pop(&seStackEntry);
					ulParam=atoi(seStackEntry.szString);

					if(ulParam>60)
						ulParam=ulParam;
					
					sprintf(seStackEntry.szString,"new Array(");

					/*elements*/ slParam=0;
					for(ulParam2=0; ulParam2<ulParam; ulParam2++)
					{
						csStack.Pop(&seStackEntry2);
						if(ulParam2) strcat(seStackEntry.szString,",");
						RemoveParens(seStackEntry2.szString);
						
						slParam+=strlen(seStackEntry2.szString);
						if(slParam<MAX_STRING-1024)
							strcat(seStackEntry.szString,seStackEntry2.szString);
						else 
							slParam=slParam;
					}

					strcat(seStackEntry.szString,")");

					csStack.Push(&seStackEntry);
					break;

					case ACTIONCODE_INITOBJECT:
					/*#elements*/
					csStack.Pop(&seStackEntry);
					ulParam=atoi(seStackEntry.szString);

					sprintf(seStackEntry.szString,"new Object(");

					/*elements*/ slParam=0;
					for(ulParam2=0; ulParam2<ulParam; ulParam2++)
					{
						csStack.Pop(&seStackEntry2);
						if(ulParam2) strcat(seStackEntry.szString,",");
						RemoveParens(seStackEntry2.szString);
						
						slParam+=strlen(seStackEntry2.szString);
						if(slParam<MAX_STRING-1024)
							strcat(seStackEntry.szString,seStackEntry2.szString);
						else 
							slParam=slParam;
					}

					strcat(seStackEntry.szString,")");

					csStack.Push(&seStackEntry);
					break;


				default: 
					sprintf(szString,"%s();\n",szAction);
			}
		}

		ulCurOffset=bsActionStream.GetByteOffset();
				
		/*check position*/
		if(ulCurOffset!=ulProgramCounter)
		{
			fprintf(pfileScript,"/*PARSE ERROR in %s*/\n\n",szAction);
			bsActionStream.Seek(ulProgramCounter);
		}

		/*print code*/
		if(szString[0])
		{
			/*if(lprgsStructs[ubLevel].ubType) 
			{
				strcat(lprgsStructs[ubLevel].szCode,szIndent);
				strcat(lprgsStructs[ubLevel].szCode,szString);
			}*/
			//else 
				fprintf(pfileScript,"%s%s",szIndent,szString);
		}

		/*check end of struct*/						
		while(	lprgsStructs[ubLevel].ubType && 
				(lprgsStructs[ubLevel].ulEnd==ulProgramCounter || 
				lprgsStructs[ubLevel].ulEnd==ulAction)
			 )		
		{
			szIndent[strlen(szIndent)-1]='\0';

			/*if(lprgsStructs[ubLevel].ubType==STRUCT_WHILE)
			{
				lprgsStructs[ubLevel].szCode
				fprintf(pfileScript,"\n%swhile",szIndent);
			}*/
			
			//fprintf(pfileScript,"%s",lprgsStructs[ubLevel].szCode);
			fprintf(pfileScript,"%s}\n\n",szIndent);
			lprgsStructs[ubLevel].ubType=0;
			//delete lprgsStructs[ubLevel].szCode;
				
			if(fStartElse)
			{
				lprgsStructs[ubLevel].ubType=STRUCT_ELSE;
				lprgsStructs[ubLevel].ulStart=ulProgramCounter;
				lprgsStructs[ubLevel].ulEnd=ulProgramCounter+slParam;
				//lprgsStructs[ubLevel].szCode=new char[MAX_STRUCT];
				//strcpy(lprgsStructs[ubLevel].szCode,"");
				fprintf(pfileScript,"%selse\n%s{\n",szIndent,szIndent);
				fStartElse=false;
			}

			else ubLevel--;
		}

		ubPrevAction=ubActionCode;
		ulAction++;
	}

	return 0;
}

/*Actions are in: DOACTION (inside frames and sprites), DEFINEBUTTON, DEFINEBUTTON2, PLACEOBJECT2*/

UI32 CSWF::SaveScript(UI16 usID)
{
	FILE *pfileScript;
	char szScriptFileName[4096];
	void **lprgvpTagArray;
	DEFINESPRITE *lpdsSprite;
	DEFINEBUTTON *lpdbButton;
	DEFINEBUTTON2 *lpdb2Button;
	BUTTONCONDACTION *lpbcaAction;
	DOACTION *lpdaDoAction;
	PLACEOBJECT2 *lppo2Place;
	CLIPACTIONS *lpcaClipActions;
	CLIPACTIONRECORD *lpcarClipAction;
	CLIPEVENTFLAGS *lpcefClipEventFlags;
	UI32 ulNumTags, iAction;
	UI16 usFrame=1;
	UI8 fNotFirst;

	UNKNOWNTAG *lputTag=(UNKNOWNTAG*)lprgvpCharacters[usID];

	if(usID && !lputTag) return 0;

	if(!usID) /*setup for movie*/
	{
		lprgvpTagArray=lprgvpTags;
		ulNumTags=nTags;

		if(!HasActions(usID)) return 0;

		CreateFileName(szScriptFileName,"script",usID,"txt");
		pfileScript=fopen(szScriptFileName,"w");
		fprintf(pfileScript,"//movie\n");
	}

	/*DEFINESPRITE*/
	else if(lputTag->rhTagHeader.usTagCode==TAGCODE_DEFINESPRITE)
	{
		lpdsSprite=(DEFINESPRITE*)lputTag;
		lprgvpTagArray=lpdsSprite->lprgvpControlTags;
		ulNumTags=lpdsSprite->ulNumControlTags;

		if(!HasActions(usID)) return 0;

		CreateFileName(szScriptFileName,"script",usID,"txt");
		pfileScript=fopen(szScriptFileName,"w");
		fprintf(pfileScript,"//sprite %lu\n",usID);
	}

	/*DEFINEBUTTON*/
	else if(lputTag->rhTagHeader.usTagCode==TAGCODE_DEFINEBUTTON)
	{
		lpdbButton=(DEFINEBUTTON*)lputTag;

		if(!lpdbButton->aaActionArray.ulNumActions) return 0;
		
		CreateFileName(szScriptFileName,"script",usID,"txt");
		pfileScript=fopen(szScriptFileName,"w");
		fprintf(pfileScript,"//button %lu (DEFINEBUTTON)\n\n",usID);
		WriteActions(&lpdbButton->aaActionArray,0,pfileScript);
		fclose(pfileScript);
		
		return 0;
	}

	/*DEFINEBUTTON2*/
	else if(lputTag->rhTagHeader.usTagCode==TAGCODE_DEFINEBUTTON2)
	{
		lpdb2Button=(DEFINEBUTTON2*)lputTag;

		if(!lpdb2Button->ulNumActions) return 0;

		CreateFileName(szScriptFileName,"script",usID,"txt");
		pfileScript=fopen(szScriptFileName,"w");
		fprintf(pfileScript,"//button %lu (DEFINEBUTTON2)\n",usID);

		for(iAction=0; iAction<lpdb2Button->ulNumActions; iAction++)
		{
			lpbcaAction=lpdb2Button->lprgbcapActions[iAction];
					
			fprintf(pfileScript,"\non(");
			fNotFirst=false;

			if(lpbcaAction->fCondOverDownToOverUp)	
			{
				if(fNotFirst) putc(',',pfileScript);
				fNotFirst=fprintf(pfileScript,"release");
			}

			if(lpbcaAction->fCondOutDownToIdle)		
			{
				if(fNotFirst) putc(',',pfileScript);
				fNotFirst=fprintf(pfileScript,"releaseOutside");
			}

			if(lpbcaAction->fCondOverUpToOverDown)
			{
				if(fNotFirst) putc(',',pfileScript);
				fNotFirst=fprintf(pfileScript,"press");
			}

			if(lpbcaAction->fCondIdleToOverDown || lpbcaAction->fCondOutDownToOverDown)
			{
				if(fNotFirst) putc(',',pfileScript);
				fNotFirst=fprintf(pfileScript,"dragOver");
			}
			
			if(lpbcaAction->fCondOverDownToOutDown || lpbcaAction->fCondOverDownToIdle) 
			{
				if(fNotFirst) putc(',',pfileScript);
				fNotFirst=fprintf(pfileScript,"dragOut");
			}

			if(lpbcaAction->fCondIdleToOverUp)
			{
				if(fNotFirst) putc(',',pfileScript);
				fNotFirst=fprintf(pfileScript,"rollOver");
			}

			if(lpbcaAction->fCondOverUpToIdle)
			{
				if(fNotFirst) putc(',',pfileScript);
				fNotFirst=fprintf(pfileScript,"rollOut");
			}

			if(lpbcaAction->ubCondKeyPress) 
			{
				if(fNotFirst) putc(',',pfileScript);
				fNotFirst=fprintf(pfileScript,"keyPress \"");

				switch(lpbcaAction->ubCondKeyPress)
				{
					case KEYCODE_LEFTARROW:		fprintf(pfileScript,"leftarrow"); break;
					case KEYCODE_RIGHTARROW:	fprintf(pfileScript,"rightarrow"); break;
					case KEYCODE_HOME:			fprintf(pfileScript,"home"); break;
					case KEYCODE_END:			fprintf(pfileScript,"end"); break;
					case KEYCODE_INSERT:		fprintf(pfileScript,"insert"); break;
					case KEYCODE_DELETE:		fprintf(pfileScript,"delete"); break;
					case KEYCODE_BACKSPACE:		fprintf(pfileScript,"backspace"); break;
					case KEYCODE_ENTER:			fprintf(pfileScript,"enter"); break;
					case KEYCODE_UPARROW:		fprintf(pfileScript,"uparrow"); break;
					case KEYCODE_DOWNARROW:		fprintf(pfileScript,"downarrow"); break;
					case KEYCODE_PAGEUP:		fprintf(pfileScript,"pageup"); break;
					case KEYCODE_PAGEDOWN:		fprintf(pfileScript,"pagedown"); break;
					case KEYCODE_TAB:			fprintf(pfileScript,"tab"); break;
					case KEYCODE_ESCAPE:		fprintf(pfileScript,"escape"); break;
					case KEYCODE_SPACEBAR:		fprintf(pfileScript,"spacebar"); break;
					default: fprintf(pfileScript,"%c",lpbcaAction->ubCondKeyPress); break;
				}

				fprintf(pfileScript,"\"");
			}

			fprintf(pfileScript,")\n{\n");

			WriteActions(&lpbcaAction->aaActionArray,1,pfileScript);
			fprintf(pfileScript,"}\n");
		}

		fclose(pfileScript);
		
		return 0;				
	}

	/*DEFINESPRITE or movie*/
	for(UI32 iTag=0; iTag<ulNumTags; iTag++)
	{
		UI16 usTagCode=((UNKNOWNTAG*)lprgvpTagArray[iTag])->rhTagHeader.usTagCode;
		
		switch(usTagCode)
		{
			case TAGCODE_DOACTION:
			case TAGCODE_DOINITACTION:
				lpdaDoAction=(DOACTION*)lprgvpTagArray[iTag];

				fprintf(pfileScript,"\n//frame %lu ",usFrame);
				if(usTagCode==TAGCODE_DOACTION) 
					fprintf(pfileScript,"(DOACTION)\n\n",usFrame);
				if(usTagCode==TAGCODE_DOINITACTION) 
					fprintf(pfileScript,"(DOINITACTION)\n\n",usFrame);

				WriteActions(&lpdaDoAction->aaActionArray,0,pfileScript);
				break;

			case TAGCODE_PLACEOBJECT2:
				lppo2Place=(PLACEOBJECT2*)lprgvpTagArray[iTag];

				if(!lppo2Place->fPlaceFlagHasClipActions) break;

				lpcaClipActions=&lppo2Place->caClipActions;

				fprintf(pfileScript,"\n//frame %lu (PLACEOBJECT2)\n\n",usFrame);

				for(iAction=0; iAction<lpcaClipActions->ulNumClipActionRecords; iAction++)
				{
					lpcarClipAction=lpcaClipActions->lprgcarpClipActionRecords[iAction];
					lpcefClipEventFlags=&lpcarClipAction->cefEventFlags;
					
					fprintf(pfileScript,"\nonClipEvent(");
					fNotFirst=false;

					
					/*19*/
					if(lpcefClipEventFlags->fClipEventKeyUp)	
					{
						if(fNotFirst) putc(',',pfileScript);
						fNotFirst=fprintf(pfileScript,"keyUp");
					}

					if(lpcefClipEventFlags->fClipEventKeyDown)	
					{
						if(fNotFirst) putc(',',pfileScript);
						fNotFirst=fprintf(pfileScript,"keyDown");
					}

					if(lpcefClipEventFlags->fClipEventMouseUp)	
					{
						if(fNotFirst) putc(',',pfileScript);
						fNotFirst=fprintf(pfileScript,"mouseUp");
					}

					if(lpcefClipEventFlags->fClipEventMouseDown)	
					{
						if(fNotFirst) putc(',',pfileScript);
						fNotFirst=fprintf(pfileScript,"mouseDown");
					}

					if(lpcefClipEventFlags->fClipEventMouseMove)	
					{
						if(fNotFirst) putc(',',pfileScript);
						fNotFirst=fprintf(pfileScript,"mouseMove");
					}

					if(lpcefClipEventFlags->fClipEventUnload)	
					{
						if(fNotFirst) putc(',',pfileScript);
						fNotFirst=fprintf(pfileScript,"unload");
					}

					if(lpcefClipEventFlags->fClipEventEnterFrame)	
					{
						if(fNotFirst) putc(',',pfileScript);
						fNotFirst=fprintf(pfileScript,"enterFrame");
					}

					if(lpcefClipEventFlags->fClipEventLoad)	
					{
						if(fNotFirst) putc(',',pfileScript);
						fNotFirst=fprintf(pfileScript,"load");
					}

					if(lpcefClipEventFlags->fClipEventDragOver)	
					{
						if(fNotFirst) putc(',',pfileScript);
						fNotFirst=fprintf(pfileScript,"DRAGOVER");
					}

					if(lpcefClipEventFlags->fClipEventRollOut)	
					{
						if(fNotFirst) putc(',',pfileScript);
						fNotFirst=fprintf(pfileScript,"ROLLOUT");
					}

					if(lpcefClipEventFlags->fClipEventRollOver)	
					{
						if(fNotFirst) putc(',',pfileScript);
						fNotFirst=fprintf(pfileScript,"ROLLOVER");
					}

					if(lpcefClipEventFlags->fClipEventReleaseOutside)	
					{
						if(fNotFirst) putc(',',pfileScript);
						fNotFirst=fprintf(pfileScript,"RELEASEOUTSIDE");
					}

					if(lpcefClipEventFlags->fClipEventRelease)	
					{
						if(fNotFirst) putc(',',pfileScript);
						fNotFirst=fprintf(pfileScript,"RELEASE");
					}

					if(lpcefClipEventFlags->fClipEventPress)	
					{
						if(fNotFirst) putc(',',pfileScript);
						fNotFirst=fprintf(pfileScript,"PRESS");
					}

					if(lpcefClipEventFlags->fClipEventInitialize)	
					{
						if(fNotFirst) putc(',',pfileScript);
						fNotFirst=fprintf(pfileScript,"INITIALIZE");
					}

					if(lpcefClipEventFlags->fClipEventData)	
					{
						if(fNotFirst) putc(',',pfileScript);
						fNotFirst=fprintf(pfileScript,"data");
					}

					if(lpcefClipEventFlags->fClipEventConstruct)	
					{
						if(fNotFirst) putc(',',pfileScript);
						fNotFirst=fprintf(pfileScript,"CONSTRUCT");
					}

					if(lpcefClipEventFlags->fClipEventKeyPress)	
					{
						if(fNotFirst) putc(',',pfileScript);
						fNotFirst=fprintf(pfileScript,"KEYPRESS");
					}

					if(lpcefClipEventFlags->fClipEventDragOut)	
					{
						if(fNotFirst) putc(',',pfileScript);
						fNotFirst=fprintf(pfileScript,"DRAGOUT");
					}

	
					fprintf(pfileScript,")\n{\n");
					WriteActions(&lpcarClipAction->aaActionArray,1,pfileScript);
					fprintf(pfileScript,"}\n");
				}
		
			case TAGCODE_SHOWFRAME: usFrame++;
		}
	}
	
	fclose(pfileScript);

	return 0;
}