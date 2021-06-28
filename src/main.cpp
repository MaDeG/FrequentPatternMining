#include <boost/program_options.hpp>
#include <queue>
#include <iostream>
#include <omp.h>
#include "FileOrderedReader.h"
#include "FPTreeManager.h"
#include "FrequentItemsets.h"
#include "Params.h"

using namespace std;

int main(int argc, char *argv[]) {
	double supportFraction;
	string input;
	bool debug, skipOutputFrequent;
	int nThreads;
	try {
		boost::program_options::options_description desc("Allowed options");
		desc.add_options()
				("help,h", "Print program usage")
				("supportFraction,s", boost::program_options::value<double>(&supportFraction)->required()->notifier([](double value) {
					if (value <= 0 || value > 100) {
						throw boost::program_options::validation_error(boost::program_options::validation_error::invalid_option_value, "supportFraction", to_string(value));
					}
				}), "Set minimum supportFraction fraction in percentage for an itemset to be considered frequent (e.g. 65%), must be a value between 0 excluded and 100 included")
				("input,i", boost::program_options::value<string>(&input)->required(), "Input file where new-line separated transactions will be read")
				("threads,t", boost::program_options::value<int>(&nThreads)->default_value(0)->notifier([](int value) {
						if (value < 0) {
							throw boost::program_options::validation_error(boost::program_options::validation_error::invalid_option_value, "nThreads", to_string(value));
						}
					}), "Number of threads to use, use 0 to use as many as the amount of cores, 1 for sequential execution, or a custom number")
				("skipOutputFrequent,o", boost::program_options::bool_switch(&skipOutputFrequent)->default_value(false), "Disables the output of the Frequent Itemsets once computed, only their count will be printed, used during performance evaluation")
				("debug,d", boost::program_options::bool_switch(&debug)->default_value(false), "Enables debug outputs");
		boost::program_options::variables_map vm;
		boost::program_options::store(boost::program_options::parse_command_line(argc, argv, desc), vm);
		if (vm.count("help")) {
			cout << desc << endl;
			return 1;
		}
		boost::program_options::notify(vm);
	} catch (exception &e) {
		cerr << e.what() << endl;
		return -1;
	}
	cout << "Input: " << input << endl;
	cout << "Support fraction: " << supportFraction << "%" << endl;
	Params::nThreads = nThreads > 0 ? nThreads : omp_get_max_threads();
	omp_set_num_threads(Params::nThreads);
	cout << "OpenMP maximum number of threads: " << Params::nThreads << endl;
	Params::debug = debug;
	DEBUG(cout << "Debug output enabled")
	omp_set_nested(true); // We would like to control nested parallelization manually

	supportFraction /= 100;

	cout << "Reading input file and computing item frequencies..." << endl;
	FileOrderedReader reader(input);
	if (!skipOutputFrequent) {
		cout << "Computed frequencies:" << endl << (string) reader << endl;
	}

	cout << "Computing Frequent Itemsets with support bigger than " << supportFraction * 100 << "%..." << endl;
	FPTreeManager<int> manager(reader, supportFraction);
	DEBUG(
		const shared_ptr<FPTreeNode<int>> root = manager.getRoot();
		cout << "FP-Tree created:" << endl << manager << endl;
		const HeaderTable<int>& headerTable = manager.getHeaderTable();
		cout << endl << headerTable << endl;
	)

	FrequentItemsets<int> frequentItemsets(manager);
	const list<list<int>> itemsets = frequentItemsets.getFrequentItemsets();
	cout << "Found " << itemsets.size() << " frequent itemsets" << endl;
	if (!skipOutputFrequent) {
		for (const list<int>& itemset : itemsets) {
			for (int item : itemset) {
				cout << item << " ";
			}
			cout << endl;
		}
	}

	return 0;
}