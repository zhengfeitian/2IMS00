#include <map>
#include <algorithm>
#include <vector>
#include <iostream>
#include <fstream>
#include <unordered_map>
#include <cstdlib>

using namespace std;

class NaiveBayesClassifer
{
	public:
	// <class id, class probility> <C, P(C)>
	unordered_map<int,double> classes_prob;
	uint32_t* classes = nullptr;
	uint32_t num_C = 0;
	uint32_t num_attr = 0;
	uint32_t attr_nv = 0;
	uint32_t* zip_ca = nullptr;
	// <class id, <attribute id, probability>> <C, <x, P(x|C)>>
	unordered_map<int, unordered_map<int, double>> attr_prob;
	uint32_t** attributePerClass = nullptr; //class_id, attr_id, count
	// input: vector< pair < class id, attribute id>> , DimSize is the number of attributes
	NaiveBayesClassifer(vector<vector<int>> &data, int DimSize, int C,int attr_nv):
		num_C(C), num_attr(DimSize), attr_nv(attr_nv)
	{

		classes = new uint32_t[C];
		memset(classes, 0, sizeof(uint32_t) * C);
		attributePerClass = new uint32_t*[C];
		for (int i = 0; i < C; i++) {
			attributePerClass[i] = new uint32_t[attr_nv];
			memset(attributePerClass[i], 0, attr_nv * sizeof(uint32_t));
		}

		zip_ca = new uint32_t[num_C+num_C*attr_nv];
	
	// start training
	// count all classes and attributes
		for(auto entry:data)
		{
			//if(classes.find(entry[0])==classes.end()){
			if(entry[0]<0 || entry[0]>=num_C){
				cout << "entry[0] " << entry[0] << endl;
				cout << "classes error" << endl;
				exit(1);
				//classes[entry[0]]=1;
				//unordered_map<int, double> pxc;
				//attributesPerClass[entry[0]] = pxc;
			}
			else{
				classes[entry[0]]+=1;
			}
			for(int k=1;k<=DimSize;k++)
			{
				//if(attributesPerClass[entry[0]].find(entry[k]) == attributesPerClass[entry[0]].end())
				if(entry[k] < 0 || entry[k]>=attr_nv)
				{
					cout << "attribute not found" << endl;
					exit(1);
					//attributesPerClass[entry[0]][entry[k]] = 1;
				}
				else
				{
					attributePerClass[entry[0]][entry[k]] += 1;
				}
			}
		}
		cout << "In constructor classes count set: " << endl;
		for(int i = 0 ; i < num_C; i++){
			cout << classes[i] << " ";
		}
		cout << endl;
	// calculate probility per class and per attribute
		/*
		for(auto seg: attributePerClass)
		{
			cout<<" - - - Class "<<seg.first<< " - - - "<< endl;
			for(auto entry: seg.second)
			{
				entry.second /= classes[seg.first];
				cout<<"Attribute P(x="<<entry.first << "| C="<<seg.first <<") = " << entry.second<< endl;
			}
			classes[seg.first] /= data.size();
			cout<<"Class P(C="<<seg.first<< ") = "<<classes[seg.first]<<endl;
		}
		*/
	}
	void zip_all_count(){
		for(int i = 0 ; i < num_C; i++){
			zip_ca[i] = classes[i];
		}
		int ind = num_C;
		for(int i = 0 ; i < num_C; i++){
			for(int j = 0 ; j < attr_nv; j++){
				zip_ca[ind] = attributePerClass[i][j];
				++ind;
			}
		}
	}
	void set_zip(uint32_t a[]){
		setClassesCount(a);
		int ind = num_C;
		for(int i = 0 ; i < num_C; i++){
			for(int j = 0 ; j < attr_nv; j++){
				attributePerClass[i][j] = zip_ca[ind];
				++ind;
			}
		}
	}
	void setClassesCount(uint32_t* sum_classes){
		for(int i = 0 ; i < num_C; i++){
			classes[i] = sum_classes[i];
		}
		cout << "classes count set: " << endl;
		for(int i = 0 ; i < num_C; i++){
			cout << classes[i] << " ";
		}
		cout << endl;
	}
	void setAttrCount(uint32_t** sum_attr){
		for(int i=0; i<num_C; i++){
			for(int j=0; j<attr_nv; j++){
				attributePerClass[i][j] = sum_attr[i][j];
			}
		}
		cout << "attr count set: " << endl;
		for(int i=0; i<num_C; i++){
			for(int j=0; j<attr_nv; j++){
				attributePerClass[i][j] = sum_attr[i][j];
				cout << attributePerClass[i][j] <<  " ";
			}
			cout << endl;
		}
			cout << endl;
	}
	void calProbability(){
		int total_classes = 0;
		for(int i = 0 ; i < num_C; i++){
			total_classes += classes[i];
		}
		for(int i=0; i<num_C; i++){
			classes_prob[i] = classes[i]/double(total_classes);
			for(int j=0; j < attr_nv; j++){
				attr_prob[i][j] = attributePerClass[i][j]/double(classes[i]);
			}
		}
	}
	// predict class with attributes vector< attribute id>
	int predict(vector<int> attributes)
	{
		int maxcid = -1;
		double maxp = 0;
		for(auto cls : classes_prob)
		{
			// p(C|x) = p(C)*p(x1|C)*p(x2|C)*…
			double pCx = cls.second;
			for(int i = 0; i<attributes.size();i++){
				pCx *= attr_prob[cls.first][attributes[i]];
			}
			
			if(pCx > maxp){
				maxp = pCx;
				maxcid = cls.first;
			}
		}
		cout<<"Predict Class: " << maxcid<< " P(C|x) = "<<maxp<<endl;
		return maxcid;
	}
};

void populateData(vector<vector<int>> &data, unordered_map<string, int> &classmap, unordered_map<string, int> &attrimap,
	string c, string a1, string a2, int K)
{
	vector<int> apair = {classmap[c],attrimap[a1], attrimap[a2]};
	vector<vector<int>> newarr(K, apair);
	data.insert(data.end(), newarr.begin(), newarr.end());
	cout << "data size: " << data.size() << endl;
	return;

}
	
void read_data(vector<vector<int>>& data, string filename,unordered_map<string,int> & classmap, 
	unordered_map<string,int> & attrimap) {
	ifstream in(filename);
	if (!in)
	{
		cerr << "Cannot open the File : " << filename << std::endl;
		return;
	}
	string c, a1, a2;
	int k;
	cout << "reading files" << endl;
	while (in >> c >> a1 >> a2 >> k) {
		populateData(data, classmap, attrimap, c, a1, a2, k);
	}
	cout << "data construction finished" << endl;
}

/*
int main() {
	// prepare a training dataset with 2 attributes and 3 classes
	unordered_map<string, int> classmap = {{"apple", 0}, {"pineapple", 1}, {"cherry", 2}};
	unordered_map<string, int> attrimap =
	// color
	{{"red", 0}, {"green", 1}, {"yellow", 2},
	// shape
	{"round", 3}, {"oval", 4}, {"heart", 5}};
	vector<vector<int>> data;
	read_data(data, "train_s.txt",classmap,attrimap);
	
	random_shuffle(data.begin(),data.end());

	// train model
	NaiveBayesClassifer mymodel(data, 2, 3, 6);
	cout << "classes : " << endl;
	for (int i = 0; i < 3; i++) {
		for (int j = 0; j < 6; j++) {
			cout << mymodel.attributePerClass[i][j] << " ";
		}
		cout << endl;
	}
	for (int i = 0; i < 3;i++) {
		cout << mymodel.classes[i] << " ";
	}
	cout << endl;
	// predict with model
	mymodel.calProbability();
	int cls = mymodel.predict({attrimap["red"],attrimap["heart"]});
	//cout<<"Predicted class "<< cls <<endl;
	return 0;
}
*/
