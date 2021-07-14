#include <iostream>
#include "FileOrderedReader.h"
#include "Params.h"

using namespace std;

FileOrderedReader::FileOrderedReader(string input) : input(input) {
	this->computeFrequencies();
}

void FileOrderedReader::computeFrequencies() {
	if (!this->input.is_open()) {
		throw invalid_argument("Cannot open the input file");
	}
	int item;
	while (this->input >> item) {
		map<int, int>::iterator lb = this->frequencies.lower_bound(item);
		// Checks whether we are performing an add or an update
		if (lb != this->frequencies.cend() && !(this->frequencies.key_comp()(item, lb->first))) {
			lb->second++;
		} else {
			this->frequencies.insert(lb, map<int, int>::value_type(item, 1));
		}
	}
	this->input.clear();
	this->input.seekg(0);
}

list<int> FileOrderedReader::getNextOrderedTransaction() {
	string line;
	list<int> itemset;
	bool ret;
	#pragma omp critical
	ret = (bool) getline(this->input, line);
	if (ret) {
		istringstream iss(line);
		itemset.assign(istream_iterator<int>(iss), istream_iterator<int>());
		itemset.sort([&](int a, int b) { return this->frequencies.at(a) > this->frequencies.at(b); });
		// We do not take into consideration duplicate elements
		itemset.unique();
		DEBUG(
				ostringstream str;
				copy(itemset.cbegin(), itemset.cend(), ostream_iterator<int>(str, " "));
				cout << "Read ordered itemset: " << str.str();
		)
	}
	return itemset; // RVO
}

bool FileOrderedReader::isEOF() const {
	return this->input.eof();
}

FileOrderedReader::operator string() const {
	ostringstream outStream;
	outStream << setw(10) << "Item" << " | " << setw(10) << "Frequency" << endl;
	for (const pair<int, int>& p : this->frequencies) {
		outStream << setw(10) << (string("'") + to_string(p.first) + string("'")) << " | " << setw(10) << p.second << endl;
	}
	return outStream.str();
}