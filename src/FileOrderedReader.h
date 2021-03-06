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
	FileOrderedReader(std::string input);
	~FileOrderedReader() = default;
	FileOrderedReader(FileOrderedReader&&) = default;
	std::list<int> getNextOrderedTransaction();
	bool isEOF() const;
	operator std::string() const;

private:
	std::ifstream input;
	std::map<int, int> frequencies;

	FileOrderedReader(const FileOrderedReader&) = default;
	void computeFrequencies();
};

inline std::ostream& operator << (std::ostream& out, const FileOrderedReader& fileOrderedReader) {
	return out << (std::string) fileOrderedReader;
}

#endif //FREQUENTPATTERNMINING_FILE_ORDERED_READER_H
