#include <boost/program_options.hpp>
#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>
#include <queue>
#include "FileOrderedReader.h"
#include "FPTreeManager.h"
#include "FrequentItemsets.h"

using namespace std;

void filter_logs() {
	boost::log::core::get()->set_filter(boost::log::trivial::severity >= boost::log::trivial::info);
}

int main(int argc, char *argv[]) {
	double supportFraction;
	string input;
	bool debug;
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
				("debug,d", boost::program_options::bool_switch(&debug)->default_value(false), "Enable or disable debug outputs");
		boost::program_options::variables_map vm;
		boost::program_options::store(boost::program_options::parse_command_line(argc, argv, desc), vm);
		if (vm.count("help")) {
			BOOST_LOG_TRIVIAL(info) << desc << "\n";
			return 1;
		}
		boost::program_options::notify(vm);
	} catch (exception &e) {
		BOOST_LOG_TRIVIAL(error) << e.what() << endl;
		return -1;
	}
	if (!debug) {
		filter_logs();
	}
	BOOST_LOG_TRIVIAL(info) << "Input: " << input;
	BOOST_LOG_TRIVIAL(info) << "Support fraction: " << supportFraction << "%";
	if (debug) {
		BOOST_LOG_TRIVIAL(info) << "Debug output enabled";
	}
	supportFraction /= 100;

	FileOrderedReader reader(input, debug);

	FPTreeManager<int> manager(reader, supportFraction, debug);
	if (debug) {
		const shared_ptr<FPTreeNode<int>> root = manager.getRoot();
		BOOST_LOG_TRIVIAL(debug) << "FP-Tree created:" << endl << manager;
		const HeaderTable<int>& headerTable = manager.getHeaderTable();
		BOOST_LOG_TRIVIAL(debug) << endl << headerTable;
	}

	/*FPTreeManager<int> newManager(manager);
	if (debug) {
		const shared_ptr<FPTreeNode<int>> root = newManager.getRoot();
		BOOST_LOG_TRIVIAL(debug) << endl << pretty_print(root);
		const HeaderTable<int>& headerTable = newManager.getHeaderTable();
		BOOST_LOG_TRIVIAL(debug) << endl << headerTable;
	}*/

	FrequentItemsets<int> frequentItemsets(manager, debug);
	shared_ptr<list<list<int>>> itemsets = frequentItemsets.getFrequentItemsets();
	BOOST_LOG_TRIVIAL(info) << "Found " << itemsets->size() << " frequent itemsets";
	for (list<int>& itemset : *itemsets) {
		string itemset_string;
		for (int item : itemset) {
			itemset_string.append(to_string(item) + " ");
		}
		BOOST_LOG_TRIVIAL(info) << itemset_string;
	}

	return 0;
}