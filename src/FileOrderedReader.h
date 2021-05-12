#ifndef FREQUENTPATTERNMINING_FILE_ORDERED_READER_H
#define FREQUENTPATTERNMINING_FILE_ORDERED_READER_H

#include <fstream>
#include <list>
#include <map>
#include <string>
#include <memory>

/**
 * Reads a file composed by new-line separated transactions, determines the
 * frequency of each item and reorders each transaction based on growing items
 * frequency.
 * This is going to be used in order to minimize the FP-tree size.
 * Warning: Multiple identical items in the same transaction will be simplified to one.
 */
class FileOrderedReader {
public:
	FileOrderedReader(std::string input, bool debug = false);
	~FileOrderedReader() = default;
	FileOrderedReader(const FileOrderedReader&) = default;
	FileOrderedReader(FileOrderedReader&&) = default;
	std::unique_ptr<std::list<int>> getNextOrderedTransaction();

private:
	std::unique_ptr<std::ifstream> _input;
	std::map<int, int> _frequencies;
	const bool _debug;

	void computeFrequencies();
};

#endif //FREQUENTPATTERNMINING_FILE_ORDERED_READER_H
