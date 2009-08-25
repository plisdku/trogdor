/*
    Trogdor FDTD
    Paul Hansen
    
 *  $Rev::                               $:  Revision of last commit
 *  $Author::                            $:  Author of last commit
 *
 *  $Date$:
 *  $Id$:

    Version History

    4.2 PLAN
    - Load modes from external files as hard sources √
    - Load modes from external files as TFSF (soft) sources [ ]
      This step was aborted because getting materials to know about having
      multiple auxiliary field buffers appears to be really complicated.  I
      will have to get this to work in a real full-on architectural revision,
      if I ever get around to that.
      This step is, however, halfway implemented through SetupLink etc.
    - Fix runlining bug (Xiaobo's bug, part II) √
    - Add some error checking in OneFieldOutput √
    - Add output option to interpolate fields for colocation

    4.1.1 (June 20, 2007)
    - Turn on SF link ability √
    - Implement fineFillRect attribute for <Block> √
    - Expose PML variables in the XML file √
    - Fix Xiaobo's bug (one-cell offset in some runline auxiliary arrays) √
    
    4.1 (February 26, 2007)
    Implemented convolutional CFS-PML for the Drude and static dielectric
    models.  Seem to work okay.  Also implemented the fillStyle attribute for
    <Tag>, <HeightMap> and <Block>, with PECStyle and PMCStyle attributes.
    Program now displays Trogdor ascii art on startup (I grabbed it from
    asciisandbox.bravehost.com).  Output is less verbose: log messages default
    to only going into the file.  Improved a few error messages.  Also, the
    output at each timestep is written using the return-carriage /r so it can
    use the same line on compatible terminals, which means not on my Mac.

    4.0  (ca. February 11, 2007)
    First "Trogdor" release.  Recent changes included moving all parameter
    file tags and attributes to Yee cell coordinates instead of half-cell
    coordinates.  Known bugs: lots of assert() statements to catch bugs, a
    mysterious late-time instability in the PML, and poor performance of the
    Drude model PML.
*/

#include "FDTDApplication.h"
#include "Version.h"
#include "Log.h"

#include "tinyxml.h"
// all I want from ImageMagick in this file is the version text, sigh.
#include <Magick++.h>

#include <boost/program_options.hpp>
#include <boost/version.hpp>

#include <iostream>
#include <limits>


using namespace std;
namespace po = boost::program_options;

po::variables_map handleArguments(int argc, char* const argv[]);

void drawTrogdor();


int main (int argc, char * const argv[])
{
	string paramFileName;
    
    SimulationPreferences prefs;
	
	// Get the command line arguments, print the help or version messages
	// as necessary, handle parsing exceptions.
	po::variables_map variablesMap = handleArguments(argc, argv);
	
	if (variablesMap.count("help") || variablesMap.count("version") ||
		variablesMap.count("numerics") )
		return 0;
		
	if (!variablesMap.count("nodragon"))
		drawTrogdor();
	
	LOGFMORE << "Trogdor version " << TROGDOR_VERSION_TEXT << endl;
	LOGFMORE << "OS type: " << TROGDOR_OS << endl;
	LOGFMORE << "Compile date: " << __DATE__ << " " << __TIME__ << endl;
	
	// The number of timesteps is specified in the parameter file.  The value
	// here is an override for debuggery.
	paramFileName = variablesMap["input-file"].as<string>();
	//directory = variablesMap["outputDirectory"].as<string>();
	//prefs.numThreads = variablesMap["numthreads"].as<int>();
	if (variablesMap.count("timesteps"))
	{
		prefs.numTimestepsOverride = variablesMap["timesteps"].as<int>();
		cout << "Running with timestep override " << prefs.numTimestepsOverride;
		cout << endl;
		LOGF << "Running with timestep override " << prefs.numTimestepsOverride;
		LOGFMORE << endl;
	}
	else
		prefs.numTimestepsOverride = -1;
    
    //LOG << variablesMap << endl;
    
	prefs.dumpGrid = variablesMap.count("dumpgrid");
	prefs.output3D = variablesMap.count("geometry");
	prefs.output2D = variablesMap.count("xsections");
	if (variablesMap.count("nosim"))
		prefs.runSim = 0;
	prefs.runSim = (variablesMap.count("nosim") == 0);
    prefs.runlineDirection = variablesMap["fastaxis"].as<char>();
    
    FDTDApplication& app = FDTDApplication::instance();
	app.runNew(paramFileName, prefs);
    
    return 0;
}


po::variables_map
handleArguments(int argc, char* const argv[])
{
	// Declare the supported options.
	// Options allowed only on the command line
	po::options_description generic("Generic");
	generic.add_options()
		("help", "produce help message")
		("version,v", "print complete version information")
		("numerics", "print numerical environment information")
	;
	
	// Options allowed on the command line or in a config file
	po::options_description config("Configuration");
	config.add_options()
		//("numthreads,n", po::value<int>()->default_value(1),
		//	"set number of concurrent threads")
		("timesteps,t", po::value<int>(), "override number of timesteps")
		("xsections,x", "write Output cross-section images")
		("geometry,g", "write 3D geometry file")
		("dumpgrid", "write compiled grid information to text files")
		("nosim", "do not run simulation")
		("nodragon", "don't draw the dragon")
        ("fastaxis,f", po::value<char>()->default_value('x'),
            "axis along which memory is allocated")
		//("outputDirectory,D",
		//	po::value<string>()->default_value(""),
		//	"directory for simulation outputs")
	;
	
	// Invisible options
	po::options_description hidden("Allowed options");
	hidden.add_options()
		("input-file", 
			po::value<string>()->default_value("params.xml"),
			"simulation description file (XML)")
	;
	
	// Group the options into pertinent sets
	po::options_description cmdlineOptions;
	cmdlineOptions.add(generic).add(config).add(hidden);
	
	po::options_description configFileOptions;
	configFileOptions.add(config).add(hidden);
	
	po::options_description visibleOptions("Allowed options");
	visibleOptions.add(generic).add(config);
	
	po::positional_options_description positional;
	positional.add("input-file", -1);
	
	po::variables_map variablesMap;
	
	try {
		po::store(po::command_line_parser(argc, const_cast<char**>(argv)).
			options(cmdlineOptions).positional(positional).run(), variablesMap);
	}
	catch (exception & e)
	{
		LOGMORE << "** There was an error parsing the command line.\n";
		LOGMORE << e.what() << endl;
		LOGMORE << "Use trogdor --help to see allowed options." << endl;
		exit(1);
	}
	
	try {
		ifstream ifs("trogdor.cfg");
		po::store(po::parse_config_file(ifs, configFileOptions), variablesMap);
	}
	catch (exception & e)
	{
		LOGMORE << "** There was an error parsing the configuration file.\n";
		LOGMORE << e.what() << endl;
		LOGMORE << "Use trogdor --help to see allowed options." << endl;
		exit(1);
	}
	
	po::notify(variablesMap);
	
	LOGF << "Command line invocation: " << endl;
	for (int nn = 0; nn < argc; nn++)
		LOGFMORE << argv[nn] << " ";
	LOGFMORE << "\n";
	LOGF << "(no more command line options)" << endl;
	// to do: dump the config file here
	LOGF << "Not dumping the config file yet.\n";
	
	if (variablesMap.count("help"))
	{
		cout << visibleOptions << "\n";
		return variablesMap;
	}
	
	if (variablesMap.count("version"))
	{
		cout << "Trogdor version: " << TROGDOR_VERSION_TEXT << endl;
		cout << "Compile date: " << __DATE__ << endl;
		cout << "OS type: " << TROGDOR_OS << endl;
		cout << "Boost version: " << BOOST_LIB_VERSION << endl;
		cout << "TinyXML version: " << TIXML_MAJOR_VERSION << "."
			<< TIXML_MINOR_VERSION << "." << TIXML_PATCH_VERSION << endl;
		cout << "ImageMagick version: " << MagickVersion << endl;
		//cout << "vmlib version: 5.1" << endl;
		cout << "calc version: 4.7" << endl;
		
		return variablesMap;
	}
	
	if (variablesMap.count("numerics"))
	{
		typedef numeric_limits<float> f;
		cout << "Values from std::limits:\n";
		cout << "digits = " << f::digits << endl;
		cout << "digits10 = " << f::digits10 << endl;
		cout << "epsilon = " << f::epsilon() << endl;
		cout << "min = " << f::min() << endl;
		cout << "min_exponent = " << f::min_exponent << endl;
		cout << "min_exponent10 = " << f::min_exponent10 << endl;
		cout << "max = " << f::max() << endl;
		cout << "max_exponent = " << f::max_exponent << endl;
		cout << "max_exponent10 = " << f::max_exponent10 << endl;
		if (f::has_denorm == denorm_present)
		{
			cout << "has_denorm = denorm_present\n";
			cout << " denorm_min = " << f::denorm_min() << endl;
			cout << " has_denorm_loss = " << f::has_denorm_loss << endl;
		}
		else if (f::has_denorm == denorm_absent)
			cout << "has_denorm = denorm_absent\n";
		else if (f::has_denorm == denorm_indeterminate)
			cout << "has_denorm = denorm_indeterminate\n";
		cout << "has_infinity = " << f::has_infinity << endl;
		cout << "has_quiet_NaN = " << f::has_quiet_NaN << endl;
		cout << "has_signaling_NaN = " << f::has_signaling_NaN << endl;
		cout << "round_error = " << f::round_error() << endl;
		cout << "round_style = ";
		switch (f::round_style)
		{
			case round_indeterminate:
				cout << "round_indeterminate\n";
				break;
			case round_toward_zero:
				cout << "round_toward_zero\n";
				break;
			case round_to_nearest:
				cout << "round_to_nearest\n";
				break;
			case round_toward_infinity:
				cout << "round_toward_infinity\n";
				break;
			case round_toward_neg_infinity:
				cout << "round_toward_neg_infinity\n";
				break;
			default:
				cout << "unknown\n";
				break;
		}
		cout << "tinyness_before = " << f::tinyness_before << endl;
		cout << "traps = " << f::traps << endl;
	}
	
	return variablesMap;
}


void drawTrogdor()
{
	cout << "\n"
	"                                                 :::\n"
	"                                             :: :::.\n"
	"                       \\/,                    .:::::\n"
	"           \\),          \\`-._                 :::888\n"
	"           /\\            \\   `-.             ::88888\n"
	"          /  \\            | .(                ::88\n"
	"         /,.  \\           ; ( `              .:8888\n"
	"            ), \\         / ;``               :::888\n"
	"           /_   \\     __/_(_                  :88\n"
	"             `. ,`..-'      `-._    \\  /      :8\n"
	"               )__ `.           `._ .\\/.\n"
	"              /   `. `             `-._______m         _,\n"
	"  ,-=====-.-;'                 ,  ___________/ _,-_,'\"`/__,-.\n"
	" C   =--   ;                   `.`._    V V V       -=-'\"#==-._\n"
	":,  \\     ,|      UuUu _,......__   `-.__A_A_ -. ._ ,--._ \",`` `-\n"
	"||  |`---' :    uUuUu,'          `'--...____/   `\" `\".   `\n"
	"|`  :       \\   UuUu:\n"
	":  /         \\   UuUu`-._\n"
	" \\(_          `._  uUuUu `-.\n"
	" (_3             `._  uUu   `._\n"
	"                    ``-._      `.\n"
	"                         `-._    `.\n"
	"                             `.    \\\n"
	"                               )   ;\n"
	"                              /   /\n"
	"               `.        |\\ ,'   /\n"
	"                 \\\",_A_/\\-| `   ,'\n"
	"                   `--..,_|_,-'\\\n"
	"                          |     \\\n"
	"                          |      \\__\n"
	"          dew             |__";
    cout << "\n\n[http://www.asciisandbox.bravehost.com]\n";
    cout << endl;
	cout << "Welcome to Trogdor version " << TROGDOR_VERSION_TEXT << "\n";
}



