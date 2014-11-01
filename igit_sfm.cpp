#include"igit_sfm.h"
#include "keys2a.h"

#include <QTextStream>
#include <QDir>
#include <QProcess>
#include <QFile>
#include <QTextStream>

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////    
void SFM::extract_focal()
{
	/****************Create Variables************************************/
	// create a hash table for ccd-width checking
	QHash<QString, float> ccd_widths;
	ccd_widths.insert( "Asahi Optical Co.,Ltd.  PENTAX Optio330RS"               , 7.176);
	ccd_widths.insert( "Canon Canon DIGITAL IXUS 400"                            , 7.176);
	ccd_widths.insert( "Canon Canon DIGITAL IXUS 40"                             , 5.76 );
	ccd_widths.insert( "Canon Canon DIGITAL IXUS 430"                            , 7.176);
	ccd_widths.insert( "Canon Canon DIGITAL IXUS 500"                            , 7.176);
	ccd_widths.insert( "Canon Canon DIGITAL IXUS 50"                             , 5.76 );
	ccd_widths.insert( "Canon Canon DIGITAL IXUS 55"                             , 5.76 );
	ccd_widths.insert( "Canon Canon DIGITAL IXUS 60"                             , 5.76 );
	ccd_widths.insert( "Canon Canon DIGITAL IXUS 65"                             , 5.76 );
	ccd_widths.insert( "Canon Canon DIGITAL IXUS 700"                            ,  7.176);
	ccd_widths.insert( "Canon Canon DIGITAL IXUS 750"                            , 7.176 );
	ccd_widths.insert( "Canon Canon DIGITAL IXUS 800 IS"                         , 5.76  );
	ccd_widths.insert( "Canon Canon DIGITAL IXUS II"                             , 5.27  );
	ccd_widths.insert( "Canon Canon EOS 10D"                                     , 22.7);
	ccd_widths.insert( "Canon Canon EOS-1D Mark II"                              , 28.7);
	ccd_widths.insert( "Canon Canon EOS-1Ds Mark II"                             , 35.95);
	ccd_widths.insert("Canon Canon EOS  20D"                                     , 22.5);
	ccd_widths.insert( "Canon Canon EOS 20D"                                     , 22.5);
	ccd_widths.insert( "Canon Canon EOS 300D DIGITAL"                            , 22.66);
	ccd_widths.insert( "Canon Canon EOS 30D"                                     , 22.5);
	ccd_widths.insert( "Canon Canon EOS 350D DIGITAL"                            , 22.2);
	ccd_widths.insert( "Canon Canon EOS 400D DIGITAL"                            , 22.2);
	ccd_widths.insert( "Canon Canon EOS 40D"                                     , 22.2);
	ccd_widths.insert( "Canon Canon EOS 5D"                                      , 35.8);
	ccd_widths.insert("Canon Canon EOS DIGITAL REBEL"                            , 22.66);
	ccd_widths.insert("Canon Canon EOS DIGITAL REBEL XT"                         , 22.2);
	ccd_widths.insert("Canon Canon EOS DIGITAL REBEL XTi"                        , 22.2);
	ccd_widths.insert( "Canon Canon EOS Kiss Digital"                            , 22.66);
	ccd_widths.insert( "Canon Canon IXY DIGITAL 600"                             , 7.176 );
	ccd_widths.insert( "Canon Canon PowerShot A20"                               , 7.176 );
	ccd_widths.insert( "Canon Canon PowerShot A400"                              , 4.54  );
	ccd_widths.insert( "Canon Canon PowerShot A40"                               , 5.27  );
	ccd_widths.insert( "Canon Canon PowerShot A510"                              , 5.76 );
	ccd_widths.insert( "Canon Canon PowerShot A520"                              , 5.76 );
	ccd_widths.insert( "Canon Canon PowerShot A530"                              , 5.76 );
	ccd_widths.insert( "Canon Canon PowerShot A60"                               , 5.27 );
	ccd_widths.insert( "Canon Canon PowerShot A620"                              , 7.176);
	ccd_widths.insert( "Canon Canon PowerShot A630"                              , 7.176);
	ccd_widths.insert( "Canon Canon PowerShot A640"                              , 7.176);
	ccd_widths.insert( "Canon Canon PowerShot A700"                              , 5.76 );
	ccd_widths.insert( "Canon Canon PowerShot A70"                               , 5.27 );
	ccd_widths.insert( "Canon Canon PowerShot A710 IS"                           , 5.76 );
	ccd_widths.insert( "Canon Canon PowerShot A75"                               , 5.27 );
	ccd_widths.insert( "Canon Canon PowerShot A95"                               , 7.176);
	ccd_widths.insert( "Canon Canon PowerShot G1"                                , 7.176);
	ccd_widths.insert( "Canon Canon PowerShot G2"                                , 7.176);
	ccd_widths.insert( "Canon Canon PowerShot G3"                                , 7.176);
	ccd_widths.insert( "Canon Canon PowerShot G5"                                , 7.176);
	ccd_widths.insert( "Canon Canon PowerShot G6"                                , 7.176);
	ccd_widths.insert( "Canon Canon PowerShot G7"                                , 7.176);
	ccd_widths.insert( "Canon Canon PowerShot G9"                                , 7.600);
	ccd_widths.insert( "Canon Canon PowerShot Pro1"                              , 8.8  );
	ccd_widths.insert( "Canon Canon PowerShot S110"                              , 5.27 );
	ccd_widths.insert( "Canon Canon PowerShot S1 IS"                             , 5.27 );
	ccd_widths.insert( "Canon Canon PowerShot S200"                              , 5.27 );
	ccd_widths.insert( "Canon Canon PowerShot S2 IS"                             , 5.76 );
	ccd_widths.insert( "Canon Canon PowerShot S30"                               , 7.176);
	ccd_widths.insert( "Canon Canon PowerShot S3 IS"                             , 5.76 );
	ccd_widths.insert( "Canon Canon PowerShot S400"                              , 7.176);
	ccd_widths.insert( "Canon Canon PowerShot S40"                               , 7.176);
	ccd_widths.insert( "Canon Canon PowerShot S410"                              , 7.176);
	ccd_widths.insert( "Canon Canon PowerShot S45"                               , 7.176);
	ccd_widths.insert( "Canon Canon PowerShot S500"                              , 7.176);
	ccd_widths.insert( "Canon Canon PowerShot S50"                               , 7.176);
	ccd_widths.insert( "Canon Canon PowerShot S60"                               , 7.176);
	ccd_widths.insert( "Canon Canon PowerShot S70"                               , 7.176);
	ccd_widths.insert( "Canon Canon PowerShot S80"                               , 7.176);
	ccd_widths.insert( "Canon Canon PowerShot SD1000"                            , 5.75 );
	ccd_widths.insert( "Canon Canon PowerShot SD100"                             , 5.27 );
	ccd_widths.insert( "Canon Canon PowerShot SD10"                              , 5.75 );
	ccd_widths.insert( "Canon Canon PowerShot SD110"                             , 5.27 );
	ccd_widths.insert( "Canon Canon PowerShot SD200"                             , 5.76 );
	ccd_widths.insert( "Canon Canon PowerShot SD300"                             , 5.76 );
	ccd_widths.insert( "Canon Canon PowerShot SD400"                             , 5.76 );
	ccd_widths.insert( "Canon Canon PowerShot SD450"                             , 5.76 );
	ccd_widths.insert( "Canon Canon PowerShot SD500"                             , 7.176);
	ccd_widths.insert( "Canon Canon PowerShot SD550"                             , 7.176);
	ccd_widths.insert( "Canon Canon PowerShot SD600"                             , 5.76 );
	ccd_widths.insert( "Canon Canon PowerShot SD630"                             , 5.76 );
	ccd_widths.insert( "Canon Canon PowerShot SD700 IS"                          , 5.76 );
	ccd_widths.insert( "Canon Canon PowerShot SD750"                             , 5.75 );
	ccd_widths.insert( "Canon Canon PowerShot SD800 IS"                          , 5.76 );
	ccd_widths.insert("Canon EOS 300D DIGITAL"                                   , 22.66);
	ccd_widths.insert( "Canon EOS DIGITAL REBEL"                                 , 22.66);
	ccd_widths.insert( "Canon PowerShot A510"                                    , 5.76 );
	ccd_widths.insert( "Canon PowerShot S30"                                     , 7.176);
	ccd_widths.insert( "CASIO COMPUTER CO.,LTD. EX-S500"                         , 5.76 );
	ccd_widths.insert( "CASIO COMPUTER CO.,LTD. EX-Z1000"                        , 7.716);
	ccd_widths.insert("CASIO COMPUTER CO.,LTD  EX-Z30"                           , 5.76 );
	ccd_widths.insert( "CASIO COMPUTER CO.,LTD. EX-Z600"                         , 5.76);
	ccd_widths.insert( "CASIO COMPUTER CO.,LTD. EX-Z60"                          , 7.176);
	ccd_widths.insert( "CASIO COMPUTER CO.,LTD  EX-Z750"                         , 7.176);
	ccd_widths.insert( "CASIO COMPUTER CO.,LTD. EX-Z850"                         , 7.176);
	ccd_widths.insert( "EASTMAN KODAK COMPANY KODAK CX7330 ZOOM DIGITAL CAMERA"  , 5.27 );
	ccd_widths.insert( "EASTMAN KODAK COMPANY KODAK CX7530 ZOOM DIGITAL CAMERA"  , 5.76 );
	ccd_widths.insert( "EASTMAN KODAK COMPANY KODAK DX3900 ZOOM DIGITAL CAMERA"  , 7.176);
	ccd_widths.insert( "EASTMAN KODAK COMPANY KODAK DX4900 ZOOM DIGITAL CAMERA"  , 7.176);
	ccd_widths.insert( "EASTMAN KODAK COMPANY KODAK DX6340 ZOOM DIGITAL CAMERA"  , 5.27 );
	ccd_widths.insert( "EASTMAN KODAK COMPANY KODAK DX6490 ZOOM DIGITAL CAMERA"  , 5.76 );
	ccd_widths.insert( "EASTMAN KODAK COMPANY KODAK DX7630 ZOOM DIGITAL CAMERA"  , 7.176);
	ccd_widths.insert( "EASTMAN KODAK COMPANY KODAK Z650 ZOOM DIGITAL CAMERA"    , 5.76  );
	ccd_widths.insert( "EASTMAN KODAK COMPANY KODAK Z700 ZOOM DIGITAL CAMERA"    , 5.76  );
	ccd_widths.insert( "EASTMAN KODAK COMPANY KODAK Z740 ZOOM DIGITAL CAMERA"    , 5.76  );
	ccd_widths.insert( "EASTMAN KODAK COMPANY KODAK Z740 ZOOM DIGITAL CAMERA"    , 5.76  );
	ccd_widths.insert( "FUJIFILM FinePix2600Zoom"                                , 5.27  );
	ccd_widths.insert( "FUJIFILM FinePix40i"                                     , 7.600);
	ccd_widths.insert( "FUJIFILM FinePix A310"                                   , 5.27 );
	ccd_widths.insert( "FUJIFILM FinePix A330"                                   , 5.27 );
	ccd_widths.insert( "FUJIFILM FinePix A600"                                   , 7.600);
	ccd_widths.insert( "FUJIFILM FinePix E500"                                   , 5.76 );
	ccd_widths.insert( "FUJIFILM FinePix E510"                                   , 5.76 );
	ccd_widths.insert( "FUJIFILM FinePix E550"                                   , 7.600);
	ccd_widths.insert( "FUJIFILM FinePix E900"                                   , 7.78 );
	ccd_widths.insert( "FUJIFILM FinePix F10"                                    , 7.600);
	ccd_widths.insert( "FUJIFILM FinePix F30"                                    , 7.600);
	ccd_widths.insert( "FUJIFILM FinePix F450"                                   , 5.76 );
	ccd_widths.insert( "FUJIFILM FinePix F601 ZOOM"                              , 7.600);
	ccd_widths.insert( "FUJIFILM FinePix S3Pro"                                  , 23.0);
	ccd_widths.insert( "FUJIFILM FinePix S5000"                                  , 5.27 );
	ccd_widths.insert( "FUJIFILM FinePix S5200"                                  , 5.76 );
	ccd_widths.insert( "FUJIFILM FinePix S5500"                                  , 5.27 );
	ccd_widths.insert( "FUJIFILM FinePix S6500fd"                                , 7.600);
	ccd_widths.insert( "FUJIFILM FinePix S7000"                                  , 7.600);
	ccd_widths.insert( "FUJIFILM FinePix Z2"                                     , 5.76 );
	ccd_widths.insert( "Hewlett-Packard hp 635 Digital Camera"                   , 4.54 );
	ccd_widths.insert( "Hewlett-Packard hp PhotoSmart 43x series"                , 5.27 );
	ccd_widths.insert( "Hewlett-Packard HP PhotoSmart 618 (V1.1)"                , 5.27 );
	ccd_widths.insert( "Hewlett-Packard HP PhotoSmart C945 (V01.61)"             , 7.176 );
	ccd_widths.insert( "Hewlett-Packard HP PhotoSmart R707 (V01.00)"             , 7.176 );
	ccd_widths.insert( "KONICA MILOLTA  DYNAX 5D"                                , 23.5  );
	ccd_widths.insert( "Konica Minolta Camera, Inc. DiMAGE A2"                   , 8.80  );
	ccd_widths.insert( "KONICA MINOLTA CAMERA, Inc. DiMAGE G400"                 , 5.76  );
	ccd_widths.insert( "Konica Minolta Camera, Inc. DiMAGE Z2"                   , 5.76  );
	ccd_widths.insert( "KONICA MINOLTA DiMAGE A200"                              , 8.80  );
	ccd_widths.insert( "KONICA MINOLTA DiMAGE X1"                                , 7.176 );
	ccd_widths.insert( "KONICA MINOLTA  DYNAX 5D"                                , 23.5  );
	ccd_widths.insert( "Minolta Co., Ltd. DiMAGE F100"                           , 7.176 );
	ccd_widths.insert( "Minolta Co., Ltd. DiMAGE Xi"                             , 5.27 );
	ccd_widths.insert( "Minolta Co., Ltd. DiMAGE Xt"                             , 5.27 );
	ccd_widths.insert( "Minolta Co., Ltd. DiMAGE Z1"                             , 5.27);
	ccd_widths.insert( "NIKON COOLPIX L3"                                        , 5.76 );
	ccd_widths.insert( "NIKON COOLPIX P2"                                        , 7.176);
	ccd_widths.insert( "NIKON COOLPIX S4"                                        , 5.76 );
	ccd_widths.insert( "NIKON COOLPIX S7c"                                       , 5.76 );
	ccd_widths.insert( "NIKON CORPORATION NIKON D100"                            , 23.7);
	ccd_widths.insert( "NIKON CORPORATION NIKON D1"                              , 23.7);
	ccd_widths.insert( "NIKON CORPORATION NIKON D1H"                             , 23.7);
	ccd_widths.insert( "NIKON CORPORATION NIKON D200"                            , 23.6);
	ccd_widths.insert( "NIKON CORPORATION NIKON D2H"                             , 23.3);
	ccd_widths.insert( "NIKON CORPORATION NIKON D2X"                             , 23.7);
	ccd_widths.insert( "NIKON CORPORATION NIKON D40"                             , 23.7);
	ccd_widths.insert( "NIKON CORPORATION NIKON D50"                             , 23.7);
	ccd_widths.insert( "NIKON CORPORATION NIKON D60"                             , 23.6);
	ccd_widths.insert( "NIKON CORPORATION NIKON D70"                             , 23.7);
	ccd_widths.insert( "NIKON CORPORATION NIKON D70s"                            , 23.7);
	ccd_widths.insert( "NIKON CORPORATION NIKON D80"                             , 23.6);
	ccd_widths.insert( "NIKON E2500"                                             , 5.27 );
	ccd_widths.insert( "NIKON E2500"                                             , 5.27 );
	ccd_widths.insert( "NIKON E3100"                                             , 5.27 );
	ccd_widths.insert( "NIKON E3200"                                             , 5.27);
	ccd_widths.insert( "NIKON E3700"                                             , 5.27 );
	ccd_widths.insert( "NIKON E4200"                                             , 7.176);
	ccd_widths.insert( "NIKON E4300"                                             , 7.18);
	ccd_widths.insert( "NIKON E4500"                                             , 7.176);
	ccd_widths.insert( "NIKON E4600"                                             , 5.76 );
	ccd_widths.insert( "NIKON E5000"                                             , 8.80 );
	ccd_widths.insert( "NIKON E5200"                                             , 7.176);
	ccd_widths.insert( "NIKON E5400"                                             , 7.176);
	ccd_widths.insert( "NIKON E5600"                                             , 5.76);
	ccd_widths.insert( "NIKON E5700"                                             , 8.80);
	ccd_widths.insert( "NIKON E5900"                                             , 7.176);
	ccd_widths.insert( "NIKON E7600"                                             , 7.176);
	ccd_widths.insert( "NIKON E775"                                              , 5.27);
	ccd_widths.insert( "NIKON E7900"                                             , 7.176);
	ccd_widths.insert( "NIKON E7900"                                             , 7.176);
	ccd_widths.insert( "NIKON E8800"                                             , 8.80);
	ccd_widths.insert( "NIKON E990"                                              , 7.176);
	ccd_widths.insert( "NIKON E995"                                              , 7.176);
	ccd_widths.insert( "NIKON S1"                                                , 5.76 );
	ccd_widths.insert( "Nokia N80"                                               , 5.27);
	ccd_widths.insert( "Nokia N80"                                               , 5.27);
	ccd_widths.insert( "Nokia N93"                                               , 4.536);
	ccd_widths.insert( "Nokia N95"                                               , 5.7 );
	ccd_widths.insert( "OLYMPUS CORPORATION     C-5000Z"                         , 7.176);
	ccd_widths.insert( "OLYMPUS CORPORATION C5060WZ"                             , 7.176);
	ccd_widths.insert( "OLYMPUS CORPORATION C750UZ"                              , 5.27);
	ccd_widths.insert( "OLYMPUS CORPORATION C765UZ"                              , 5.76);
	ccd_widths.insert( "OLYMPUS CORPORATION C8080WZ"                             , 8.80);
	ccd_widths.insert( "OLYMPUS CORPORATION X250,D560Z,C350Z"                    , 5.76);
	ccd_widths.insert( "OLYMPUS CORPORATION     X-3,C-60Z"                       , 7.176);
	ccd_widths.insert( "OLYMPUS CORPORATION X400,D580Z,C460Z"                    , 5.27);
	ccd_widths.insert( "OLYMPUS IMAGING CORP.   E-500"                           , 17.3);
	ccd_widths.insert(   "OLYMPUS IMAGING CORP.   FE115,X715"                    , 5.76);
	ccd_widths.insert( "OLYMPUS IMAGING CORP. SP310"                             , 7.176);
	ccd_widths.insert( "OLYMPUS IMAGING CORP.   SP510UZ"                         , 5.75);
	ccd_widths.insert( "OLYMPUS IMAGING CORP.   SP550UZ"                         , 5.76);
	ccd_widths.insert( "OLYMPUS IMAGING CORP.   uD600,S600"                      , 5.75 );
	ccd_widths.insert( "OLYMPUS_IMAGING_CORP.   X450,D535Z,C370Z"                , 5.27 );
	ccd_widths.insert( "OLYMPUS IMAGING CORP. X550,D545Z,C480Z"                  , 5.76 );
	ccd_widths.insert( "OLYMPUS OPTICAL CO.,LTD C2040Z"                          , 6.40);
	ccd_widths.insert( "OLYMPUS OPTICAL CO.,LTD C211Z"                           , 5.27 );
	ccd_widths.insert( "OLYMPUS OPTICAL CO.,LTD C2Z,D520Z,C220Z"                 , 4.54);
	ccd_widths.insert( "OLYMPUS OPTICAL CO.,LTD C3000Z"                          , 7.176);
	ccd_widths.insert( "OLYMPUS OPTICAL CO.,LTD C300Z,D550Z"                     , 5.4);
	ccd_widths.insert( "OLYMPUS OPTICAL CO.,LTD C4100Z,C4000Z"                   , 7.176);
	ccd_widths.insert( "OLYMPUS OPTICAL CO.,LTD C750UZ"                          , 5.27);
	ccd_widths.insert( "OLYMPUS OPTICAL CO.,LTD X-2,C-50Z"                       , 7.176);
	ccd_widths.insert( "OLYMPUS SP550UZ"                                         , 5.76);
	ccd_widths.insert( "OLYMPUS X100,D540Z,C310Z"                                , 5.27 );
	ccd_widths.insert( "Panasonic DMC-FX01"                                      , 5.76 );
	ccd_widths.insert( "Panasonic DMC-FX07"                                      , 5.75 );
	ccd_widths.insert( "Panasonic DMC-FX9"                                       , 5.76 );
	ccd_widths.insert( "Panasonic DMC-FZ20"                                      , 5.760);
	ccd_widths.insert( "Panasonic DMC-FZ2"                                       , 4.54 );
	ccd_widths.insert( "Panasonic DMC-FZ30"                                      , 7.176);
	ccd_widths.insert( "Panasonic DMC-FZ50"                                      , 7.176);
	ccd_widths.insert( "Panasonic DMC-FZ5"                                       , 5.760);
	ccd_widths.insert( "Panasonic DMC-FZ7"                                       , 5.76 );
	ccd_widths.insert( "Panasonic DMC-LC1"                                       , 8.80 );
	ccd_widths.insert( "Panasonic DMC-LC33"                                      , 5.760);
	ccd_widths.insert( "Panasonic DMC-LX1"                                       , 8.50 );
	ccd_widths.insert( "Panasonic DMC-LZ2"                                       , 5.76 );
	ccd_widths.insert( "Panasonic DMC-TZ1"                                       , 5.75 );
	ccd_widths.insert( "Panasonic DMC-TZ3"                                       , 5.68 );
	ccd_widths.insert( "PENTAX Corporation  PENTAX *ist DL"                      , 23.5);
	ccd_widths.insert( "PENTAX Corporation  PENTAX *ist DS2"                     , 23.5);
	ccd_widths.insert( "PENTAX Corporation  PENTAX *ist DS"                      , 23.5);
	ccd_widths.insert( "PENTAX Corporation  PENTAX K100D"                        , 23.5);
	ccd_widths.insert( "PENTAX Corporation PENTAX Optio 450"                     , 7.176);
	ccd_widths.insert( "PENTAX Corporation PENTAX Optio 550"                     , 7.176);
	ccd_widths.insert( "PENTAX Corporation PENTAX Optio E10"                     , 5.76);
	ccd_widths.insert( "PENTAX Corporation PENTAX Optio S40"                     , 5.76 );
	ccd_widths.insert( "PENTAX Corporation  PENTAX Optio S4"                     , 5.76);
	ccd_widths.insert( "PENTAX Corporation PENTAX Optio S50"                     , 5.76 );
	ccd_widths.insert( "PENTAX Corporation  PENTAX Optio S5i"                    , 5.76);
	ccd_widths.insert( "PENTAX Corporation  PENTAX Optio S5z"                    , 5.76);
	ccd_widths.insert( "PENTAX Corporation  PENTAX Optio SV"                     , 5.76);
	ccd_widths.insert( "PENTAX Corporation PENTAX Optio WP"                      , 5.75);
	ccd_widths.insert( "RICOH CaplioG3 modelM"                                   , 5.27 );
	ccd_widths.insert( "RICOH       Caplio GX"                                   , 7.176);
	ccd_widths.insert( "RICOH       Caplio R30"                                  , 5.75);
	ccd_widths.insert( "Samsung  Digimax 301"                                    , 5.27);
	ccd_widths.insert( "Samsung Techwin <Digimax i5, Samsung #1>"                , 5.76);
	ccd_widths.insert( "SAMSUNG TECHWIN Pro 815"                                 , 8.80);
	ccd_widths.insert( "SONY DSC-F828"                                           , 8.80 );
	ccd_widths.insert( "SONY DSC-N12"                                            , 7.176);
	ccd_widths.insert( "SONY DSC-P100"                                           , 7.176);
	ccd_widths.insert( "SONY DSC-P10"                                            , 7.176);
	ccd_widths.insert( "SONY DSC-P12"                                            , 7.176);
	ccd_widths.insert( "SONY DSC-P150"                                           , 7.176);
	ccd_widths.insert( "SONY DSC-P200"                                           , 7.176);
	ccd_widths.insert( "SONY DSC-P52"                                            , 5.27 );
	ccd_widths.insert( "SONY DSC-P72"                                            , 5.27 );
	ccd_widths.insert( "SONY DSC-P73"                                            , 5.27);
	ccd_widths.insert( "SONY DSC-P8"                                             , 5.27 );
	ccd_widths.insert( "SONY DSC-R1"                                             , 21.5);
	ccd_widths.insert( "SONY DSC-S40"                                            , 5.27 );
	ccd_widths.insert( "SONY DSC-S600"                                           , 5.760);
	ccd_widths.insert( "SONY DSC-T9"                                             , 7.18);
	ccd_widths.insert( "SONY DSC-V1"                                             , 7.176);
	ccd_widths.insert( "SONY DSC-W1"                                             , 7.176);
	ccd_widths.insert( "SONY DSC-W30"                                            , 5.760);
	ccd_widths.insert( "SONY DSC-W50"                                            , 5.75 );
	ccd_widths.insert( "SONY DSC-W5"                                             , 7.176);
	ccd_widths.insert( "SONY DSC-W7"                                             , 7.176);
	ccd_widths.insert( "SONY DSC-W80"                                            , 5.75 );


	QString  outputText;

	/***************Extract Exif Information**************************/
	// create out folder
	QString jhead("jhead.exe");
	int num_output_images = 0;
	QVector<QString> lines_to_write;

	foreach(QString img_dir, s_img_dirs_)
	{
		QProcess cmd;
		cmd.start(jhead.toStdString().c_str(), QStringList()<<img_dir);

		outputText = tr("[Extracting exif tags from image")  + img_dir + tr("]");
		emit textEdit(outputText);

		while(cmd.waitForFinished())
		{
			//perform jhead.exe to obtain the whole exif information
			QString result(cmd.readAllStandardOutput());
			QStringList exif_info = result.split("\n");

			QString camera_make("");
			QString camera_model("");
			float scale =1.0;
			float focal_mm =0.0;
			float ccd_width_mm =0.0;
			bool has_focal=false;
			float res_x = 0.0;
			float res_y = 0.0;

			// Camera make
			foreach(QString tmp, exif_info)
			{
				if(tmp.startsWith("Camera make  :"))
				{
					camera_make = tmp;
					int id= camera_make.indexOf(":");
					camera_make.remove(0, id+1);
					camera_make = camera_make.trimmed();
				}
			}

			// Camera model
			foreach(QString tmp, exif_info)
			{
				if(tmp.startsWith("Camera model"))
				{
					camera_model = tmp;
					int id= camera_model.indexOf(":");
					camera_model.remove(0, id+1);
					camera_model = camera_model.trimmed();
				}
			}

			//focal length
			foreach(QString tmp, exif_info)
			{
				if(tmp.startsWith("Focal length"))
				{
					QString focal_str = tmp;
					int id1= focal_str.indexOf(":");
					focal_str.remove(0, id1+1);
					int id2= focal_str.indexOf("mm");
					focal_str.remove(id2, focal_str.length());
					focal_str = focal_str.trimmed();
					focal_mm = focal_str.toFloat();
				}
			}
			//std::cout<<" [Focal length = "<< focal_mm<<"mm]"<<endl;
			outputText = QString(" [Focal length = %1 mm]").arg(focal_mm);
			emit textEdit(outputText);

			//CCD Width
			QString str = camera_make + " " + camera_model;
			//cout<<str.toStdString()<<endl;
			ccd_width_mm = ccd_widths[str];
			if(ccd_width_mm==0)
			{
				/**get from jhead**/
				// get ccd width from exif
				foreach(QString tmp, exif_info)
				{
					if(tmp.startsWith("CCD width"))
					{
						QString ccdWidth_str = tmp;

						// find the position of ":"
						int id1= ccdWidth_str.indexOf(":");
						ccdWidth_str.remove(0, id1+1);

						// find the position of "mm"
						int id2= ccdWidth_str.indexOf("mm");
						ccdWidth_str.remove(id2, ccdWidth_str.length());

						// elinimate " " from head and "\n" from end
						ccdWidth_str = ccdWidth_str.trimmed();

						// convert string to float
						ccd_width_mm = ccdWidth_str.toFloat();
					}
				}

				if(ccd_width_mm!=0)
				{
					//std::cout<<"[Found in EXIF tags]"<<endl;
					outputText = QString("[Found in EXIF tags]").arg(focal_mm);
					emit textEdit(outputText);
				}
			}
			//std::cout<<"[CCD width = "<< ccd_width_mm<<" mm]"<<endl;
			outputText = QString("[CCD width = %1 mm]").arg(ccd_width_mm);
			emit textEdit(outputText);

			// resolution
			res_x = 0.0;
			res_y = 0.0;
			foreach(QString tmp, exif_info)
			{
				if(tmp.startsWith("Resolution"))
				{
					QStringList tmp_list = tmp.split(" ");

					// find the position of "x"
					int id = 0;
					for(int i=0; i< tmp_list.size();i++)
					{
						// cout<< tmp_list[i].toStdString()<<endl;
						if(tmp_list[i]== "x")
						{
							id =i;
						}
					}

					// convert string to float
					if(id -1 >=0)
					{
						res_x = tmp_list[id -1].toFloat();
					}
					if(id+1 <tmp_list.length())
					{
						res_y = tmp_list[id +1] .toFloat();
					}
				}
			}
			//std::cout<<" [Resolution= " << res_x <<" x "<< res_y<<"]"<<endl;
			outputText = QString(" [Resolution= %1 x %2 ]").arg(res_x).arg(res_y);
			emit textEdit(outputText);

			// has focal
			if(focal_mm==0||ccd_width_mm==0||res_x==0)
			{
				has_focal = false;
			}
			else{
				has_focal = true;
			}

			// process resolution
			if(res_x < res_y)
			{
				float tmp = res_x;
				res_x = res_y;
				res_y = tmp;
			}

			//get base name
			QString basename;
			QStringList tmp_list = img_dir.split(".");
			basename = tmp_list.takeFirst();

			// lines to write
			QString line;
			if(has_focal == true)
			{
				float focal_pixels = res_x*(focal_mm/ccd_width_mm);
				//std::cout<<"[Focal length (pixels) = "<< focal_pixels<<"]"<<endl;
				line = QString("%1.jpg 0 %2").arg(basename).arg(scale*focal_pixels);
			}
			else{
				line = QString("%1.jpg").arg(basename);
			}

			lines_to_write<<line;
			num_output_images++;

		}// while
		emit textEdit(tr("\n"));
	}
	//std::cout<<"[Found "<<num_output_images <<" good images]"<<endl;
	outputText = QString("[Found %1 good images]").arg(num_output_images);
	emit textEdit( outputText);

	/*****************Write files *******************************/

	QString out_file_dir =tr( "list.txt");

	QFile out_file(out_file_dir);
	if( !out_file.open(QIODevice::WriteOnly))
	{
		//std::cout<<"Fail to create "<< out_file_dir.toStdString()<<endl;
		outputText = tr("Fail to create ")+ out_file_dir;
		emit textEdit(outputText);
	}
	else{
		QTextStream out(&out_file);
		foreach(QString line, lines_to_write)
		{
			out<<line<<endl;
		}
	}
}
//////////////////////////////////////////extract_feature////////////////////////////////////////////////////////////////////////////////////////
void SFM::extract_feature()
{
	QString  outputText = tr("[Extracting Features...\n");
	emit textEdit( outputText);

	int nImgs = s_img_dirs_.size();

	QVector<QString> featFileList;
	foreach(QString img_dir, s_img_dirs_)
	{
		outputText = tr("Extracting Features From ") + img_dir;
		emit textEdit(outputText);

		int index = img_dir.lastIndexOf('.');
		QString featFile = img_dir.mid(0, index+1) + tr("key");
		featFileList.push_back(featFile);

		QDir dir(s_img_folder_);
		if(dir.exists(featFile))
		{
			outputText = QString("KeyPoints File Existed!\n");
			emit textEdit(outputText);
			continue;
		}

		//estract featrures
		int key_num = extract_vlsift(img_dir.toLocal8Bit(), featFile.toLocal8Bit(), true); // sift
		//int key_num = extract_vlmrogh(img_dir.toLocal8Bit(), featFile.toLocal8Bit());/// features from fan

		outputText = QString("Extracted %1 KeyPoints\n").arg(key_num);
		emit textEdit(outputText);
	}

	outputText = tr("Done! ] \n");
	emit textEdit(outputText);

	// create files list_keys
	QFile out_file("list_keys.txt");
	if( !out_file.open(QIODevice::WriteOnly))
	{
		//std::cout<<"Fail to create "<< out_file_dir.toStdString()<<endl;
		QString outputText = tr("Fail to create list_keys.txt");
		emit textEdit(outputText);
	}
	else{
		QTextStream out(&out_file);
		foreach(QString featFile, featFileList)
		{
			out<<featFile<<endl;
		}
	}
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void SFM::feature_matching()
{
	QString outputText = tr("[Features Matching...");
	emit textEdit(outputText);

	KepMatchFull("list_keys.txt", "matches.init.txt");

	outputText = tr(" Done!]\n");
	emit textEdit(outputText);
}
//////////////////////////////////////////////// Bundler/////////////////////////////////////////////////////////
void SFM::bundler()
{
	QString outputText = tr("[Run Bundler...");
	emit textEdit(outputText);

	QDir dir(QDir::currentPath());
	if(dir.exists("bundle"))
	{
		dir.rmdir("bundle");
	}
	dir.mkdir("bundle");

	FILE* fid = fopen("./options.txt","wt");
	fprintf(fid, "--match_table matches.init.txt\n");
	fprintf(fid, "--output bundle.out\n");
	fprintf(fid, "--output_all bundle_\n");
	fprintf(fid, "--output_dir bundle\n");
	fprintf(fid, "--variable_focal_length\n");
	fprintf(fid, "--use_focal_estimate\n");
	fprintf(fid, "--constrain_focal\n");
	fprintf(fid, "--constrain_focal_weight 0.0001\n");
	fprintf(fid, "--estimate_distortion\n");
	fprintf(fid, "--run_bundle\n");
	fclose(fid);

	system("Bundler.exe list.txt --options_file options.txt > bundler.out");

#if 0
	QProcess cmd;
	cmd.start("Bundler.exe list.txt --options_file options.txt > bundler.out");
	while(cmd.waitForFinished())
	{
	}
#endif
	outputText = tr(" Done!]\n");

	remove("bundle.out");
	remove("constraints.txt");
	remove("matches.corresp.txt");
	remove("matches.ransac.txt");
	remove("nmatches.corresp.txt");
	remove("nmatches.ransac.txt");
	remove("pairwise_scores.txt");
	remove("matches.prune.txt");
	remove("nmatches.prune.txt");
	remove("track-pairs.txt");
	emit textEdit(outputText);
}
////////////////////////////////////////////////Run /////////////////////////////////////////////////////////////
void SFM::run()
{

	emit statusBar("Extract Exif Information");
	extract_focal();
	emit statusBar("Extract Exif Information Done!");

	// Extract Features
	emit statusBar("Extract Features");
	extract_feature();
	emit statusBar("Extract Features Done!");

	// Features Matching 
	emit statusBar("Features Matching");
	feature_matching();
	emit statusBar("Done!");

	// Run Bundler
	emit statusBar("Run Bundler");
	bundler();
	emit statusBar("Done!");

	emit statusBar("Collect Sparse Points");
	collect_sparse_points();
	emit statusBar("Done!");

	emit enableActionSparse();
}
///////////////////////////////////////////////collect_sparse_points//////////////////////////////////////////
void SFM::collect_sparse_points()
{
	s_sparse_pts_->clear();

	QDir dir;
	if(dir.exists(tr("bundle")))
	{
		QString path = QDir::currentPath() + tr("/bundle");
		dir.setPath(path);
		dir.setFilter(QDir::Files | QDir::NoSymLinks | QDir::NoDotAndDotDot);
		QStringList filter;
		filter<<"*.ply";

		// ply files
		QFileInfoList fileList = dir.entryInfoList(filter);
		int nFiles = fileList.size();	
		// the last file containes all the points
		QString ply_file_name = fileList.at(nFiles-1).filePath();

		//load points
		loadPointsFromPLYFile(ply_file_name);
	}
}
//////////////////////////////////////loadPointsFromPLYFile/////////////////////////////////////////////////
bool SFM::loadPointsFromPLYFile(QString file_name)
{
	QFile file(file_name);
	if(!file.open(QIODevice::ReadOnly))
	{
		emit textEdit(tr("Fail to Load Sparse Points!"));
		return false;
	}

	QTextStream in(&file);
	int line_Num = 0;
	while(!in.atEnd())
	{
		QString line = in.readLine();
		QStringList fields = line.split(" ");

		// points begin
		if(line_Num>=12)
		{
			Point pt;

			pt.x = fields.takeFirst().toFloat();
            pt.y = fields.takeFirst().toFloat();
			pt.z = fields.takeFirst().toFloat();

			pt.r = fields.takeFirst().toInt();
			pt.g = fields.takeFirst().toInt();
			pt.b = fields.takeFirst().toInt();

			s_sparse_pts_->append(pt);
		}

		line_Num++;
	}

	return true;
}