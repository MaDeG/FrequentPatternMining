#include <boost/log/trivial.hpp>
#include <sstream>
#include "FileOrderedReader.h"

using namespace std;

FileOrderedReader::FileOrderedReader(string input, bool debug) : debug(debug) {
	this->input = make_unique<ifstream>(input);
	this->computeFrequencies();
	BOOST_LOG_TRIVIAL(info) << "Computed frequencies:";
	for (const pair<int, int> &p : this->frequencies) {
		BOOST_LOG_TRIVIAL(info) << "Item '" << p.first << "'\tfrequency: " << p.second;
	}
}

void FileOrderedReader::computeFrequencies() {
	if (!this->input->is_open()) {
		throw invalid_argument("Cannot open the input file");
	}
	int item;
	while (*this->input >> item) {
		map<int, int>::iterator lb = this->frequencies.lower_bound(item);
		// Checks whether we are performing an add or an update
		if (lb != this->frequencies.cend() && !(this->frequencies.key_comp()(item, lb->first))) {
			lb->second++;
		} else {
			this->frequencies.insert(lb, map<int, int>::value_type(item, 1));
		}
	}
	this->input->clear();
	this->input->seekg(0);
}

unique_ptr<list<int>> FileOrderedReader::getNextOrderedTransaction() {
	string line;
	if (!getline(*this->input, line)) {
		return unique_ptr<list<int>>();
	}
	unique_ptr<list<int>> itemset = make_unique<list<int>>();
	istringstream iss(line);
	itemset->assign(istream_iterator<int>(iss), std::istream_iterator<int>());
	itemset->sort([&](int a, int b) { return this->frequencies.at(a) > this->frequencies.at(b); });
	// We do not take into consideration duplicate elements
	itemset->unique();
	if (this->debug) {
		ostringstream str;
		copy(itemset->cbegin(), itemset->cend(), ostream_iterator<int>(str, " "));
		BOOST_LOG_TRIVIAL(debug) << "Read ordered itemset: " << str.str();
	}
	return move(itemset);
}