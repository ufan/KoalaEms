dnl ***************************************************************************
dnl                                                                           *
dnl modultypes.inc                                                            *
dnl  R. Nellen                                               03.03.1993       *
dnl                                                                           *
dnl $ZEL: modultypes.inc,v 2.54 2016/04/28 21:52:11 wuestner Exp $
dnl                                                                           *
dnl ***************************************************************************
dnl
dnl          Module_Id fuer COSY Experiment Hardware
dnl
dnl
dnl  8 Character jeweils von 0 bis f zur Darstellung im HEX Format
dnl
dnl
dnl  -----------------------------------------------------------------
dnl  |       |       |       |       |       |       |       |       |
dnl  |   7   |   6   |   5   |   4   |   3   |   2   |   1   |   0   |
dnl  |       |       |       |       |       |       |       |       |
dnl  -----------------------------------------------------------------
dnl    |        |        |   \___________________  __________________/
dnl    |        |        |                       \/
dnl  BUS-Type Function Factory                Module Type
dnl
dnl  1=CAMAC  1=ADC    1=LeCroy       e.g.   4300b
dnl  2=VME    2=QDC    2=Silena              04301
dnl  3=DL350  3=TDC    3=Struck              043fc ==> FSC
dnl  4=Fastb. 4=Scaler 4=Kinetics            043be ==> Bitpattern
dnl  5=LVD    5=Flash  5=Philips
dnl  6=CANbus 6=Contr. 6=CES
dnl  7=IP     7=Memory 7=GSI
dnl           8=Logic  8=CERN
dnl           9=Bitpat 9=CAEN
dnl           a=Trig.M a=SIS
dnl           b=Power  b=FZJ/ZEL
dnl           c=CPU    c=ISEG
dnl           d=Discr. d=EG&G ORTEC
dnl           e=Seq.   e=INFN
dnl           f=other  f=other
dnl                    f0=JORWAY
dnl                    f1=mesytec
dnl                    f2=Joerger
dnl                    f3=opaque commands
dnl
dnl  in case of Factory==0xf nibble 4 is used for factory too
dnl    Module Type can then use nibble 0..3 only

module(no_module         ,0)

dnl CAMAC

module(FERA_ADC_4300B    ,0x1214300B)
module(FERA_TDC_4300B    ,0x1314300B)
module(FERA_DRV_4301     ,0x16104301)
module(FERA_MEM_4302     ,0x17104302)
module(FERA_TFC_4303     ,0x1f104303)
module(FERA_TDC_3377     ,0x13103377)
module(FSC               ,0x16f043FC)
module(BIT_PATTERN_UNIT  ,0x19f043BE)
module(LC_TDC_2277       ,0x13102277)
module(LC_TDC_2228       ,0x1312228A)
module(LC_TDC_2229       ,0x13102229)
module(LC_QDC_2249       ,0x1212249A)
module(SILENA_ADC_4418V  ,0x11204418)
module(CNRS_QDC_1612F    ,0x12F1612F)
module(CNRS_TDC_812F     ,0x13F0812F)
module(RAL11X_RDOUT      ,0x18f0011a)

module(LC_SCALER_2551    ,0x14102551)
module(LC_SCALER_4434    ,0x14104434)
module(VCC2117           ,0x1c602117)

module(GSI_TRIGGER       ,0x1A70C100)
module(DRAMS_RECEIVER    ,0x1180D512)
module(DRAMS_CONTROL     ,0x168000DC)
module(DRAMS_EVENT       ,0x18800ECD)

module(SCALER_JEA20      ,0x19000010)
module(CES_HSM2170       ,0x19000011)
module(CES_HM2161        ,0x19000012)
module(IM_JIR10          ,0x19000013)

module(CAEN_DISC_C193    ,0x1d90C193)
module(CAEN_DISC_C808    ,0x1d90C808)
module(CAEN_DISC_C894    ,0x1d90C894)
module(LC_LOGIC_4516     ,0x18104516)
module(LC_DISC_4413      ,0x1d104413)
module(LC_DISC_4416      ,0x1d104416)
module(LC_DISC_3420      ,0x1d103420)
module(LC_REGISTER_2371  ,0x1f102371)
module(LC_MLU_2373       ,0x18102373)
module(LC_MALU_4532      ,0x18104532)
module(LC_LOGIC_4564     ,0x18104564)
module(LC_DELAY_4418     ,0x18104418)
module(LC_DELAY_4518     ,0x18104518)
module(LC_8LM_2365       ,0x18102365)
module(LC_PLU_4508       ,0x18104508)

module(CAENNET_SLOW      ,0x1690c139)

module(IGOR              ,0x1f403060)
module(CAEN_PROG_IO      ,0x1890C219)

module(SIS_PreScaler     ,0x14305802)
module(CAEN_MLU_C85      ,0x1890C085)
module(CAEN_MLU_C86      ,0x1890C086)
module(ORTEC_DISC_CCF8200,0x1dd08200)
module(PH_DISC_7106      ,0x1d507106)
module(PH_ADC_7164       ,0x11507164)
module(PH_CONV_7126      ,0x1f507126)
module(BiRa_IN_2351      ,0x19f02351)
module(JW_TIME_SEQ_221   ,0x1ef00221)
module(JW_LEMO_221L      ,0x1ff00221)
module(JR_Clock          ,0x1ff20010)

dnl FASTBUS

module(PH_ADC_10C2       ,0x415010C2)
module(PH_TDC_10C6       ,0x435010C6)
module(STR_330           ,0x4C3068A0)
module(GSI_TRIGGER_FB    ,0x4A70F100)
module(FVSBI             ,0x466068B7)
module(KIN_ADC_F465      ,0x4140100A)
module(KIN_TDC_F432      ,0x43401008)
module(LC_CAT_1810       ,0x48101039)
module(LC_TDC_1872A      ,0x43101036)
module(LC_TDC_1875       ,0x43101049)
module(LC_TDC_1875A      ,0x43101037)
module(LC_TDC_1876       ,0x4310103B)
module(LC_TDC_1877       ,0x4310103C)
module(LC_TDC_1879       ,0x43101035)
module(LC_ADC_1881       ,0x4110104B)
module(LC_ADC_1881M      ,0x4110104F)
module(LC_ADC_1885F      ,0x41101045)
module(STR_197           ,0x4F306854)

dnl VME

module(FIC_8232          ,0x2C608232)
module(FIC_8234          ,0x2C608234)
module(ELTEC_E6          ,0x2CF000E6)
module(GSI_TRIGGER_VME   ,0x2A70E100)
module(CES_8251          ,0x2F608251)
module(VPG517            ,0x26f02474)
module(SIS_3100          ,0x26a31000)
module(SIS_3300          ,0x21a33000)
module(SIS_3301          ,0x21a33010)
module(SIS_3316          ,0x21a33160)
module(SIS_3400          ,0x23a34000)
module(SIS_3400clock     ,0x23a3400c)
module(SIS_3600          ,0x29a36000)
module(SIS_3601          ,0x29a36010)
module(SIS_3700          ,0x29a37000)
module(SIS_3800          ,0x24a38000)
module(SIS_3801          ,0x24a38010)
module(SIS_3802          ,0x24a38020)
module(SIS_3803          ,0x24a38030)
module(SIS_3804          ,0x24a38040)
module(SIS_3805          ,0x24a38050)
module(SIS_3806          ,0x24a38060)
module(SIS_3807          ,0x2fa38070)
module(SIS_3808          ,0x24a38080)
module(SIS_3809          ,0x24a38090)
module(SIS_3810          ,0x24a38100)
module(SIS_3811          ,0x24a38110)
module(SIS_3820          ,0x24a38200)
module(STR_340           ,0x2f303400)
module(SIS_4100          ,0x2fa41000)
module(VMIC8X16BITDAC    ,0x2ff04116)
module(CAEN_V259         ,0x29902590)
module(CAEN_V259E        ,0x2990259e)
module(CAEN_V262         ,0x28902620)
module(CAEN_V265         ,0x22902650)
module(CAEN_V462         ,0x2f904620)
module(CAEN_V512         ,0x28905120)
module(CAEN_V513         ,0x29905130)
module(CAEN_V550         ,0x21905500)
module(CAEN_V550A        ,0x2190550a)
module(CAEN_V551B        ,0x2690551b)
module(CAEN_V556         ,0x21905560)
module(CAEN_V560         ,0x24905600)
module(CAEN_V673         ,0x23906730)
module(CAEN_V676         ,0x21906760)
module(CAEN_V693         ,0x23906930)
module(CAEN_V729         ,0x21907290)
module(CAEN_V729A        ,0x2190729a)
module(CAEN_V767         ,0x23907670)
module(CAEN_V775         ,0x23907750)
module(CAEN_V785         ,0x21907850)
module(CAEN_V792         ,0x22907920)
module(CAEN_V820         ,0x24908200)
module(CAEN_V830         ,0x24908300)
module(CAEN_VN1488       ,0x23914880)
module(CAEN_V1190        ,0x23911900)
module(CAEN_V1290        ,0x23912900)
module(ZEL_SYNC_VME      ,0x2AB00010)
module(INFN_TRIGCONTR    ,0x26e02475)
module(CAEN_V1495        ,0x28914950)
module(mesytec_madc32    ,0x21f10032)
module(mesytec_mqdc32    ,0x22f10032)
module(mesytec_mtdc32    ,0x23f10032)
module(vme_mcst          ,0x2ffffff0)
module(vme_cblt          ,0x2ffffff1)

dnl ZEL LVD

module(ZEL_LVD_UNKNOWN           ,0x5ff00000)
module(ZEL_LVD_INVALID           ,0x5ff00001)
module(ZEL_LVD_CONTROLLER_GPX    ,0x56b00030)
module(ZEL_LVD_CONTROLLER_F1     ,0x56b00010)
module(ZEL_LVD_CONTROLLER_F1GPX  ,0x56b00040)
dnl module(ZEL_LVD_SLAVE_CONTROLLER  ,0x56b00020)
module(ZEL_LVD_TDC_F1            ,0x53b00010)
module(ZEL_LVD_TDC_GPX           ,0x53b00020)
module(ZEL_LVD_ADC_VERTEX        ,0x51b00010)
module(ZEL_LVD_ADC_VERTEXM3      ,0x51b00020)
module(ZEL_LVD_ADC_VERTEXUN      ,0x51b00030)
module(ZEL_LVD_MSYNCH            ,0x5ab00010)
module(ZEL_LVD_OSYNCH            ,0x5ab00020)
module(ZEL_LVD_FQDC              ,0x52b00010)
module(ZEL_LVD_VFQDC             ,0x52b00030)
module(ZEL_LVD_SQDC              ,0x52b00020)
module(ZEL_LVD_TRIGGER           ,0x58b00010)

dnl ISEG power supplies (CANbus)
dnl (only modul classes)

module(ISEG_POWER_0      ,0x6bc00000)
module(ISEG_POWER_1      ,0x6bc00001)
module(ISEG_POWER_2      ,0x6bc00002)
module(ISEG_POWER_3      ,0x6bc00003)
module(ISEG_POWER_6      ,0x6bc00006)
module(ISEG_POWER_7      ,0x6bc00007)

dnl ????

module(NE526             ,526)
module(E217              ,217)
module(E362              ,362)
module(ESSI              ,422)
module(LED_2800          ,2800)
module(ROC717            ,717)
module(E233_0            ,2330)
module(E233_1            ,2331)
module(CHANNEL           ,123)
module(ILL_CHANNEL       ,30)
module(IEC               ,100)

dnl device server over IP; arbitrary (text) commands

module(IP_opaque         ,0x7ff30001)

dnl ***************************************************************************
dnl ***************************************************************************
