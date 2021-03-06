/******************************************************************************\
 * Copyright (c) 2004-2020
 *
 * Author(s):
 *  Volker Fischer
 *
 ******************************************************************************
 *
 * This program is free software; you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation; either version 2 of the License, or (at your option) any later 
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT 
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more 
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program; if not, write to the Free Software Foundation, Inc., 
 * 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA
 *
\******************************************************************************/

#include <QCoreApplication>
#include <QDir>
#include <QTextStream>
#include <QTranslator>
#include <QLibraryInfo>
#include "global.h"
#ifndef HEADLESS
# include <QApplication>
# include <QMessageBox>
# include "clientdlg.h"
# include "serverdlg.h"
#endif
#include "settings.h"
#include "testbench.h"
#include "util.h"
#ifdef ANDROID
# include <QtAndroidExtras/QtAndroid>
#endif
#if defined ( __APPLE__ ) || defined ( __MACOSX )
# include "mac/activity.h"
#endif


// Implementation **************************************************************

int main ( int argc, char** argv )
{

    QTextStream& tsConsole = *( ( new ConsoleWriterFactory() )->get() );
    QString      strArgument;
    double       rDbleArgument;

    // initialize all flags and string which might be changed by command line
    // arguments
#if defined( SERVER_BUNDLE ) && ( defined( __APPLE__ ) || defined( __MACOSX ) )
    // if we are on MacOS and we are building a server bundle, starts Jamulus in server mode
    bool         bIsClient                   = false;
#else
    bool         bIsClient                   = true;
#endif
    bool         bUseGUI                     = true;
    bool         bStartMinimized             = false;
    bool         bShowComplRegConnList       = false;
    bool         bDisconnectAllClientsOnQuit = false;
    bool         bUseDoubleSystemFrameSize   = true; // default is 128 samples frame size
    bool         bShowAnalyzerConsole        = false;
    bool         bCentServPingServerInList   = false;
    bool         bNoAutoJackConnect          = false;
    bool         bUseTranslation             = true;
    bool         bCustomPortNumberGiven      = false;
    int          iNumServerChannels          = DEFAULT_USED_NUM_CHANNELS;
    int          iMaxDaysHistory             = DEFAULT_DAYS_HISTORY;
    int          iCtrlMIDIChannel            = INVALID_MIDI_CH;
    quint16      iPortNumber                 = DEFAULT_PORT_NUMBER;
    ELicenceType eLicenceType                = LT_NO_LICENCE;
    QString      strConnOnStartupAddress     = "";
    QString      strIniFileName              = "";
    QString      strHTMLStatusFileName       = "";
    QString      strServerName               = "";
    QString      strLoggingFileName          = "";
    QString      strHistoryFileName          = "";
    QString      strRecordingDirName         = "";
    QString      strCentralServer            = "";
    QString      strServerInfo               = "";
    QString      strWelcomeMessage           = "";
    QString      strClientName               = APP_NAME;

    // QT docu: argv()[0] is the program name, argv()[1] is the first
    // argument and argv()[argc()-1] is the last argument.
    // Start with first argument, therefore "i = 1"
    for ( int i = 1; i < argc; i++ )
    {
        // Server mode flag ----------------------------------------------------
        if ( GetFlagArgument ( argv,
                               i,
                               "-s",
                               "--server" ) )
        {
            bIsClient = false;
            tsConsole << "- server mode chosen" << endl;
            continue;
        }


        // Use GUI flag --------------------------------------------------------
        if ( GetFlagArgument ( argv,
                               i,
                               "-n",
                               "--nogui" ) )
        {
            bUseGUI = false;
            tsConsole << "- no GUI mode chosen" << endl;
            continue;
        }


        // Use licence flag ----------------------------------------------------
        if ( GetFlagArgument ( argv,
                               i,
                               "-L",
                               "--licence" ) )
        {
            // right now only the creative commons licence is supported
            eLicenceType = LT_CREATIVECOMMONS;
            tsConsole << "- licence required" << endl;
            continue;
        }


        // Use 64 samples frame size mode ----------------------------------------------------
        if ( GetFlagArgument ( argv,
                               i,
                               "-F",
                               "--fastupdate" ) )
        {
            bUseDoubleSystemFrameSize = false; // 64 samples frame size
            tsConsole << "- using " << SYSTEM_FRAME_SIZE_SAMPLES << " samples frame size mode" << endl;
            continue;
        }


        // Maximum number of channels ------------------------------------------
        if ( GetNumericArgument ( tsConsole,
                                  argc,
                                  argv,
                                  i,
                                  "-u",
                                  "--numchannels",
                                  1,
                                  MAX_NUM_CHANNELS,
                                  rDbleArgument ) )
        {
            iNumServerChannels = static_cast<int> ( rDbleArgument );

            tsConsole << "- maximum number of channels: "
                << iNumServerChannels << endl;

            continue;
        }


        // Maximum days in history display -------------------------------------
        if ( GetNumericArgument ( tsConsole,
                                  argc,
                                  argv,
                                  i,
                                  "-D",
                                  "--histdays",
                                  1,
                                  366,
                                  rDbleArgument ) )
        {
            iMaxDaysHistory = static_cast<int> ( rDbleArgument );

            tsConsole << "- maximum days in history display: "
                << iMaxDaysHistory << endl;

            continue;
        }


        // Start minimized -----------------------------------------------------
        if ( GetFlagArgument ( argv,
                               i,
                               "-z",
                               "--startminimized" ) )
        {
            bStartMinimized = true;
            tsConsole << "- start minimized enabled" << endl;
            continue;
        }


        // Ping servers in list for central server -----------------------------
        if ( GetFlagArgument ( argv,
                               i,
                               "-g",
                               "--pingservers" ) )
        {
            bCentServPingServerInList = true;
            tsConsole << "- ping servers in slave server list" << endl;
            continue;
        }


        // Disconnect all clients on quit --------------------------------------
        if ( GetFlagArgument ( argv,
                               i,
                               "-d",
                               "--discononquit" ) )
        {
            bDisconnectAllClientsOnQuit = true;
            tsConsole << "- disconnect all clients on quit" << endl;
            continue;
        }


        // Disabling auto Jack connections -------------------------------------
        if ( GetFlagArgument ( argv,
                               i,
                               "-j",
                               "--nojackconnect" ) )
        {
            bNoAutoJackConnect = true;
            tsConsole << "- disable auto Jack connections" << endl;
            continue;
        }


        // Disable translations ------------------------------------------------
        if ( GetFlagArgument ( argv,
                               i,
                               "-t",
                               "--notranslation" ) )
        {
            bUseTranslation = false;
            tsConsole << "- translations disabled" << endl;
            continue;
        }


        // Show all registered servers in the server list ----------------------
        // Undocumented debugging command line argument: Show all registered
        // servers in the server list regardless if a ping to the server is
        // possible or not.
        if ( GetFlagArgument ( argv,
                               i,
                               "--showallservers", // no short form
                               "--showallservers" ) )
        {
            bShowComplRegConnList = true;
            tsConsole << "- show all registered servers in server list" << endl;
            continue;
        }


        // Show analyzer console -----------------------------------------------
        // Undocumented debugging command line argument: Show the analyzer
        // console to debug network buffer properties.
        if ( GetFlagArgument ( argv,
                               i,
                               "--showanalyzerconsole", // no short form
                               "--showanalyzerconsole" ) )
        {
            bShowAnalyzerConsole = true;
            tsConsole << "- show analyzer console" << endl;
            continue;
        }


        // Controller MIDI channel ---------------------------------------------
        if ( GetNumericArgument ( tsConsole,
                                  argc,
                                  argv,
                                  i,
                                  "--ctrlmidich", // no short form
                                  "--ctrlmidich",
                                  0,
                                  15,
                                  rDbleArgument ) )
        {
            iCtrlMIDIChannel = static_cast<int> ( rDbleArgument );
            tsConsole << "- selected controller MIDI channel: " << iCtrlMIDIChannel << endl;
            continue;
        }


        // Use logging ---------------------------------------------------------
        if ( GetStringArgument ( tsConsole,
                                 argc,
                                 argv,
                                 i,
                                 "-l",
                                 "--log",
                                 strArgument ) )
        {
            strLoggingFileName = strArgument;
            tsConsole << "- logging file name: " << strLoggingFileName << endl;
            continue;
        }


        // Port number ---------------------------------------------------------
        if ( GetNumericArgument ( tsConsole,
                                  argc,
                                  argv,
                                  i,
                                  "-p",
                                  "--port",
                                  0,
                                  65535,
                                  rDbleArgument ) )
        {
            iPortNumber            = static_cast<quint16> ( rDbleArgument );
            bCustomPortNumberGiven = true;
            tsConsole << "- selected port number: " << iPortNumber << endl;
            continue;
        }


        // HTML status file ----------------------------------------------------
        if ( GetStringArgument ( tsConsole,
                                 argc,
                                 argv,
                                 i,
                                 "-m",
                                 "--htmlstatus",
                                 strArgument ) )
        {
            strHTMLStatusFileName = strArgument;
            tsConsole << "- HTML status file name: " << strHTMLStatusFileName << endl;
            continue;
        }

        if ( GetStringArgument ( tsConsole,
                                 argc,
                                 argv,
                                 i,
                                 "-a",
                                 "--servername",
                                 strArgument ) )
        {
            strServerName = strArgument;
            tsConsole << "- server name for HTML status file: " << strServerName << endl;
            continue;
        }


        // Client Name ---------------------------------------------------------
        if ( GetStringArgument ( tsConsole,
                                 argc,
                                 argv,
                                 i,
                                 "--clientname",
                                 "--clientname",
                                 strArgument ) )
        {
            strClientName = QString ( APP_NAME ) + " " + strArgument;
            tsConsole << "- client name: " << strClientName << endl;
            continue;
        }


        // Server history file name --------------------------------------------
        if ( GetStringArgument ( tsConsole,
                                 argc,
                                 argv,
                                 i,
                                 "-y",
                                 "--history",
                                 strArgument ) )
        {
            strHistoryFileName = strArgument;
            tsConsole << "- history file name: " << strHistoryFileName << endl;
            continue;
        }


        // Recording directory -------------------------------------------------
        if ( GetStringArgument ( tsConsole,
                                 argc,
                                 argv,
                                 i,
                                 "-R",
                                 "--recording",
                                 strArgument ) )
        {
            strRecordingDirName = strArgument;
            tsConsole << "- recording directory name: " << strRecordingDirName << endl;
            continue;
        }


        // Central server ------------------------------------------------------
        if ( GetStringArgument ( tsConsole,
                                 argc,
                                 argv,
                                 i,
                                 "-e",
                                 "--centralserver",
                                 strArgument ) )
        {
            strCentralServer = strArgument;
            tsConsole << "- central server: " << strCentralServer << endl;
            continue;
        }


        // Server info ---------------------------------------------------------
        if ( GetStringArgument ( tsConsole,
                                 argc,
                                 argv,
                                 i,
                                 "-o",
                                 "--serverinfo",
                                 strArgument ) )
        {
            strServerInfo = strArgument;
            tsConsole << "- server info: " << strServerInfo << endl;
            continue;
        }


        // Server welcome message ----------------------------------------------
        if ( GetStringArgument ( tsConsole,
                                 argc,
                                 argv,
                                 i,
                                 "-w",
                                 "--welcomemessage",
                                 strArgument ) )
        {
            strWelcomeMessage = strArgument;
            tsConsole << "- welcome message: " << strWelcomeMessage << endl;
            continue;
        }


        // Initialization file -------------------------------------------------
        if ( GetStringArgument ( tsConsole,
                                 argc,
                                 argv,
                                 i,
                                 "-i",
                                 "--inifile",
                                 strArgument ) )
        {
            strIniFileName = strArgument;
            tsConsole << "- initialization file name: " << strIniFileName << endl;
            continue;
        }


        // Connect on startup --------------------------------------------------
        if ( GetStringArgument ( tsConsole,
                                 argc,
                                 argv,
                                 i,
                                 "-c",
                                 "--connect",
                                 strArgument ) )
        {
            strConnOnStartupAddress = strArgument;
            tsConsole << "- connect on startup to address: " << strConnOnStartupAddress << endl;
            continue;
        }


        // Version number ------------------------------------------------------
        if ( ( !strcmp ( argv[i], "--version" ) ) ||
             ( !strcmp ( argv[i], "-v" ) ) )
        {
            tsConsole << GetVersionAndNameStr ( false ) << endl;
            exit ( 1 );
        }


        // Help (usage) flag ---------------------------------------------------
        if ( ( !strcmp ( argv[i], "--help" ) ) ||
             ( !strcmp ( argv[i], "-h" ) ) ||
             ( !strcmp ( argv[i], "-?" ) ) )
        {
            const QString strHelp = UsageArguments ( argv );
            tsConsole << strHelp << endl;
            exit ( 1 );
        }


        // Unknown option ------------------------------------------------------
        tsConsole << argv[0] << ": ";
        tsConsole << "Unknown option '" <<
            argv[i] << "' -- use '--help' for help" << endl;

// clicking on the Mac application bundle, the actual application
// is called with weird command line args -> do not exit on these
#if !( defined ( __APPLE__ ) || defined ( __MACOSX ) )
        exit ( 1 );
#endif
    }

#ifdef HEADLESS
    if ( bUseGUI )
    {
        bUseGUI = false;
        tsConsole << "No GUI support compiled. Running in headless mode." << endl;
    }
    Q_UNUSED ( bStartMinimized )       // avoid compiler warnings
    Q_UNUSED ( bShowComplRegConnList ) // avoid compiler warnings
    Q_UNUSED ( bShowAnalyzerConsole )  // avoid compiler warnings
#endif


    // Dependencies ------------------------------------------------------------
    // per definition: if we are in "GUI" server mode and no central server
    // address is given, we use the default central server address
    if ( !bIsClient && bUseGUI && strCentralServer.isEmpty() )
    {
        strCentralServer = DEFAULT_SERVER_ADDRESS;
    }

    // adjust default port number for client: use different default port than the server since
    // if the client is started before the server, the server would get a socket bind error
    if ( bIsClient && !bCustomPortNumberGiven )
    {
        iPortNumber += 10; // increment by 10
    }

    // display a warning if in server no GUI mode and a history file is requested
    if ( !bIsClient && !bUseGUI && !strHistoryFileName.isEmpty() )
    {
        tsConsole << "Qt5 requires a windowing system to paint a JPEG image; image will use SVG" << endl;
    }
    
    // Application/GUI setup ---------------------------------------------------
    // Application object
#ifdef HEADLESS
    QCoreApplication* pApp = new QCoreApplication ( argc, argv );
#else
    QCoreApplication* pApp = bUseGUI
        ? new QApplication ( argc, argv )
        : new QCoreApplication ( argc, argv );
#endif

#ifdef ANDROID
    // special Android coded needed for record audio permission handling
    auto result = QtAndroid::checkPermission ( QString ( "android.permission.RECORD_AUDIO" ) );

    if ( result == QtAndroid::PermissionResult::Denied )
    {
        QtAndroid::PermissionResultMap resultHash = QtAndroid::requestPermissionsSync ( QStringList ( { "android.permission.RECORD_AUDIO" } ) );

        if ( resultHash["android.permission.RECORD_AUDIO"] == QtAndroid::PermissionResult::Denied )
        {
            return 0;
        }
    }
#endif

#ifdef _WIN32
    // set application priority class -> high priority
    SetPriorityClass ( GetCurrentProcess(), HIGH_PRIORITY_CLASS );

    // For accessible support we need to add a plugin to qt. The plugin has to
    // be located in the install directory of the software by the installer.
    // Here, we set the path to our application path.
    QDir ApplDir ( QApplication::applicationDirPath() );
    pApp->addLibraryPath ( QString ( ApplDir.absolutePath() ) );
#endif

#if defined ( __APPLE__ ) || defined ( __MACOSX )
    // On OSX we need to declare an activity to ensure the process doesn't get
    // throttled by OS level Nap, Sleep, and Thread Priority systems.
    CActivity activity;

    activity.BeginActivity();
#endif

    // init resources
    Q_INIT_RESOURCE(resources);

    // load translations
    QTranslator myappTranslator, myqtTranslator;

    if ( bUseGUI && bUseTranslation )
    {
        if ( myappTranslator.load ( QLocale(), "translation", "_", ":/translations" ) )
        {
            pApp->installTranslator ( &myappTranslator );
        }

        // allows the Qt messages to be translated in the application
        if ( myqtTranslator.load ( QLocale(), "qt", "_", QLibraryInfo::location ( QLibraryInfo::TranslationsPath ) ) )
        {
            pApp->installTranslator ( &myqtTranslator );
        }
    }


// TEST -> activate the following line to activate the test bench,
//CTestbench Testbench ( "127.0.0.1", DEFAULT_PORT_NUMBER );


    try
    {
        if ( bIsClient )
        {
            // Client:
            // actual client object
            CClient Client ( iPortNumber,
                             strConnOnStartupAddress,
                             iCtrlMIDIChannel,
                             bNoAutoJackConnect,
                             strClientName );

            // load settings from init-file
            CClientSettings Settings ( &Client, strIniFileName );
            Settings.Load();

#ifndef HEADLESS
            if ( bUseGUI )
            {
                // GUI object
                CClientDlg ClientDlg ( &Client,
                                       &Settings,
                                       strConnOnStartupAddress,
                                       iCtrlMIDIChannel,
                                       bShowComplRegConnList,
                                       bShowAnalyzerConsole,
                                       nullptr,
                                       Qt::Window );

                // show dialog
                ClientDlg.show();
                pApp->exec();
            }
            else
#endif
            {
                // only start application without using the GUI
                tsConsole << GetVersionAndNameStr ( false ) << endl;

                pApp->exec();
            }
        }
        else
        {
            // Server:
            // actual server object
            CServer Server ( iNumServerChannels,
                             iMaxDaysHistory,
                             strLoggingFileName,
                             iPortNumber,
                             strHTMLStatusFileName,
                             strHistoryFileName,
                             strServerName,
                             strCentralServer,
                             strServerInfo,
                             strWelcomeMessage,
                             strRecordingDirName,
                             bCentServPingServerInList,
                             bDisconnectAllClientsOnQuit,
                             bUseDoubleSystemFrameSize,
                             eLicenceType );

#ifndef HEADLESS
            if ( bUseGUI )
            {
                // load settings from init-file
                CServerSettings Settings ( &Server, strIniFileName );
                Settings.Load();

                // update server list AFTER restoring the settings from the
                // settings file
                Server.UpdateServerList();

                // GUI object for the server
                CServerDlg ServerDlg ( &Server,
                                       &Settings,
                                       bStartMinimized,
                                       nullptr,
                                       Qt::Window );

                // show dialog (if not the minimized flag is set)
                if ( !bStartMinimized )
                {
                    ServerDlg.show();
                }

                pApp->exec();
            }
            else
#endif
            {
                // only start application without using the GUI
                tsConsole << GetVersionAndNameStr ( false ) << endl;

                // update serverlist
                Server.UpdateServerList();

                pApp->exec();
            }
        }
    }

    catch ( const CGenErr& generr )
    {
        // show generic error
#ifndef HEADLESS
        if ( bUseGUI )
        {
            QMessageBox::critical ( nullptr,
                                    APP_NAME,
                                    generr.GetErrorText(),
                                    "Quit",
                                    nullptr );
        }
        else
#endif
        {
            tsConsole << generr.GetErrorText() << endl;
        }
    }
    
    #if defined ( __APPLE__ ) || defined ( __MACOSX )
        activity.EndActivity();
    #endif

    return 0;
}


/******************************************************************************\
* Command Line Argument Parsing                                                *
\******************************************************************************/
QString UsageArguments ( char **argv )
{
    return
        "Usage: " + QString ( argv[0] ) + " [option] [optional argument]\n"
        "\nRecognized options:\n"
        "  -h, -?, --help        display this help text and exit\n"
        "  -i, --inifile         initialization file name\n"
        "  -n, --nogui           disable GUI\n"
        "  -p, --port            set your local port number\n"
        "  -t, --notranslation   disable translation (use englisch language)\n"
        "  -v, --version         output version information and exit\n"
        "\nServer only:\n"
        "  -a, --servername      server name, required for HTML status\n"
        "  -d, --discononquit    disconnect all clients on quit\n"
        "  -D, --histdays        number of days of history to display\n"
        "  -e, --centralserver   address of the central server\n"
        "  -F, --fastupdate      use 64 samples frame size mode\n"
        "  -g, --pingservers     ping servers in list to keep NAT port open\n"
        "                        (central server only)\n"
        "  -l, --log             enable logging, set file name\n"
        "  -L, --licence         a licence must be accepted on a new\n"
        "                        connection\n"
        "  -m, --htmlstatus      enable HTML status file, set file name\n"
        "  -o, --serverinfo      infos of the server(s) in the format:\n"
        "                        [name];[city];[country as QLocale ID]; ...\n"
        "                        [server1 address];[server1 name]; ...\n"
        "                        [server1 city]; ...\n"
        "                        [server1 country as QLocale ID]; ...\n"
        "                        [server2 address]; ...\n"
        "  -R, --recording       enables recording and sets directory to contain\n"
        "                        recorded jams\n"
        "  -s, --server          start server\n"
        "  -u, --numchannels     maximum number of channels\n"
        "  -w, --welcomemessage  welcome message on connect\n"
        "  -y, --history         enable connection history and set file name\n"
        "  -z, --startminimized  start minimizied\n"
        "\nClient only:\n"
        "  -c, --connect         connect to given server address on startup\n"
        "  -j, --nojackconnect   disable auto Jack connections\n"
        "  --ctrlmidich          MIDI controller channel to listen\n"
        "  --clientname          client name (window title and jack client name)\n"
        "\nExample: " + QString ( argv[0] ) + " -s --inifile myinifile.ini\n";
}

bool GetFlagArgument ( char**  argv,
                       int&    i,
                       QString strShortOpt,
                       QString strLongOpt )
{
    if ( ( !strShortOpt.compare ( argv[i] ) ) ||
         ( !strLongOpt.compare ( argv[i] ) ) )
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool GetStringArgument ( QTextStream& tsConsole,
                         int          argc,
                         char**       argv,
                         int&         i,
                         QString      strShortOpt,
                         QString      strLongOpt,
                         QString&     strArg )
{
    if ( ( !strShortOpt.compare ( argv[i] ) ) ||
         ( !strLongOpt.compare ( argv[i] ) ) )
    {
        if ( ++i >= argc )
        {
            tsConsole << argv[0] << ": ";
            tsConsole << "'" << strLongOpt << "' needs a string argument" << endl;
            exit ( 1 );
        }

        strArg = argv[i];

        return true;
    }
    else
    {
        return false;
    }
}

bool GetNumericArgument ( QTextStream& tsConsole,
                          int          argc,
                          char**       argv,
                          int&         i,
                          QString      strShortOpt,
                          QString      strLongOpt,
                          double       rRangeStart,
                          double       rRangeStop,
                          double&      rValue )
{
    if ( ( !strShortOpt.compare ( argv[i] ) ) ||
         ( !strLongOpt.compare ( argv[i] ) ) )
    {
        if ( ++i >= argc )
        {
            tsConsole << argv[0] << ": ";

            tsConsole << "'" <<
                strLongOpt << "' needs a numeric argument between " <<
                rRangeStart << " and " << rRangeStop << endl;

            exit ( 1 );
        }

        char *p;
        rValue = strtod ( argv[i], &p );
        if ( *p ||
             ( rValue < rRangeStart ) ||
             ( rValue > rRangeStop ) )
        {
            tsConsole << argv[0] << ": ";

            tsConsole << "'" <<
                strLongOpt << "' needs a numeric argument between " <<
                rRangeStart << " and " << rRangeStop << endl;

            exit ( 1 );
        }

        return true;
    }
    else
    {
        return false;
    }
}
