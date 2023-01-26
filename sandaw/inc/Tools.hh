#include <vector>
#include <iostream>
#include <algorithm>

void MergeWindows(std::vector<int64_t>& start, std::vector<int64_t>& end,
	std::vector<int64_t>& iWS, std::vector<int64_t>& iWE) {

	int64_t l = start[0], r = end[0]; //current left and current right

	for (int i = 0; i < start.size()-1; i++) {
		if ((start[i + 1] - r) < 0) {
			if (end[i + 1] > r) {
				r = end[i + 1];
			}
		}
		else {
			iWS.push_back(l);
			iWE.push_back(r);
			l = start[i + 1];
			r = end[i + 1];
		}
	}
	
	iWS.push_back(l);
	iWE.push_back(r);

}


template<typename T>
void LargestTwoArgs(const std::vector<T> &array, int iNSort, int* LargestTwo) {
    std::vector<unsigned int> indices(iNSort);
    std::iota(indices.begin(), indices.end(), 0);
    std::sort(indices.begin(), indices.end(),
              [&array](int left, int right) -> bool {
                  // sort indices according to corresponding array element
                  return array[left] < array[right];
              });

    LargestTwo[0] = indices[iNSort-1];
	LargestTwo[1] = indices[iNSort-2];
}