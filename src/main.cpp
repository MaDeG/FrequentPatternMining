#include <queue>
#include <boost/program_options.hpp>
#include <boost/log/core.hpp>
#include <boost/log/trivial.hpp>
#include <boost/log/expressions.hpp>
#include <iostream>
#include "FileOrderedReader.h"
#include "FPTreeManager.h"

using namespace std;

void filter_logs() {
	boost::log::core::get()->set_filter(boost::log::trivial::severity >= boost::log::trivial::info);
}

string pretty_print(const map<int, shared_ptr<FPTreeNode<int>>> &table) {
	string out = "Key\t\t-\t\tNode\n";
	for (const map<int, shared_ptr<FPTreeNode<int>>>::value_type& i : table) {
		out.append(to_string(i.first) + "\t\t-\t\t" + ((string) *i.second) + "\n");
	}
	return out;
}

string pretty_print(const FPTreeNode<int>& root) {
	string out;
	int currentLevel = 0;
	queue<const FPTreeNode<int>*> nodes;
	queue<int> levels;
	nodes.push(&root);
	levels.push(0);
	while(!nodes.empty()) {
		assert(nodes.size() == levels.size());
		const FPTreeNode<int>* node = nodes.front();
		const int level = levels.front();
		nodes.pop();
		levels.pop();
		if (currentLevel != level) {
			assert(currentLevel == level - 1);
			out.append("\n");
			currentLevel = level;
			if (node == nullptr) {
				continue;
			}
		} else if (node == nullptr) {
			out.append("\t\t|\t\t");
			continue;
		}
		out.append(((string) *node) + " - ");
		for (shared_ptr<FPTreeNode<int>> i : node->getChildren()) {
			nodes.push(i.get());
			levels.push(level + 1);
		}
		nodes.push(nullptr);
		levels.push(level + 1);
	}
	return out;
}

int main(int argc, char *argv[]) {
	double support;
	string input;
	bool debug;
	try {
		boost::program_options::options_description desc("Allowed options");
		desc.add_options()
				("help,h", "Print program usage")
				("support,s", boost::program_options::value<double>(&support)->required(), "Set minimum support in percentage for an itemset to be considered frequent (e.g. 65%)")
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
	BOOST_LOG_TRIVIAL(info) << "Support: " << support << "%";
	if (debug) {
		BOOST_LOG_TRIVIAL(info) << "Debug output enabled";
	}

	FileOrderedReader reader(input, debug);

	FPTreeManager<int> manager(reader);
	if (debug) {
		const FPTreeNode<int>& root = manager.getRoot();
		BOOST_LOG_TRIVIAL(debug) << endl << pretty_print(root);
		const map<int, shared_ptr<FPTreeNode<int>>>& headerTable = manager.getHeaderTable();
		BOOST_LOG_TRIVIAL(debug) << endl << pretty_print(headerTable);
	}

	return 0;
}