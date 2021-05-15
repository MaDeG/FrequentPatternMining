#include <boost/log/trivial.hpp>
#include <sstream>
#include "FileOrderedReader.h"

using namespace std;

FileOrderedReader::FileOrderedReader(string input, bool debug) : _debug(debug) {
	this->_input = make_unique<ifstream>(input);
	this->computeFrequencies();
	BOOST_LOG_TRIVIAL(info) << "Computed frequencies:";
	for (const pair<int, int> &p : this->_frequencies) {
		BOOST_LOG_TRIVIAL(info) << "Item '" << p.first << "'\tfrequency: " << p.second;
	}
}

void FileOrderedReader::computeFrequencies() {
	if (!this->_input->is_open()) {
		throw invalid_argument("Cannot open the input file");
	}
	int item;
	while (*this->_input >> item) {
		map<int, int>::iterator lb = this->_frequencies.lower_bound(item);
		// Checks whether we are performing an add or an update
		if (lb != this->_frequencies.end() && !(this->_frequencies.key_comp()(item, lb->first))) {
			lb->second++;
		} else {
			this->_frequencies.insert(lb, map<int, int>::value_type(item, 1));
		}
	}
	this->_input->clear();
	this->_input->seekg(0);
}

unique_ptr<list<int>> FileOrderedReader::getNextOrderedTransaction() {
	string line;
	if (!getline(*this->_input, line)) {
		return unique_ptr<list<int>>();
	}
	unique_ptr<list<int>> itemset = make_unique<list<int>>();
	istringstream iss(line);
	itemset->assign(istream_iterator<int>(iss), std::istream_iterator<int>());
	itemset->sort([&](int a, int b) { return this->_frequencies.at(a) > this->_frequencies.at(b); });
	// We do not take into consideration duplicate elements
	itemset->unique();
	if (this->_debug) {
		ostringstream str;
		copy(itemset->begin(), itemset->end(), ostream_iterator<int>(str, " "));
		BOOST_LOG_TRIVIAL(debug) << "Read ordered itemset: " << str.str();
	}
	return itemset;
}