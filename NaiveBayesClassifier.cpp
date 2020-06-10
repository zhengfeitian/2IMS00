#include <map>
#include <algorithm>
#include <vector>
#include <iostream>
#include <fstream>
#include <map>
#include <cstdlib>
#include <sstream>
#include <stdio.h>  // defines FILENAME_MAX
#include <filesystem>

using namespace std;

class NaiveBayesClassifer
{
	public:
	// <class id, class probility> <C, P(C)>
	map<int,double> classes_prob;
	uint32_t* classes = nullptr;
	uint32_t num_C = 0;
	uint32_t num_attr = 0;
	uint32_t attr_nv = 0;
	uint32_t* zip_ca = nullptr;
	// <class id, <attribute id, probability>> <C, <x, P(x|C)>>
	map<int, map<int, double>> attr_prob;
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
				//map<int, double> pxc;
				//attributesPerClass[entry[0]] = pxc;
			}
			else{
				classes[entry[0]]+=1;
			}
			for(int k=1;k<=DimSize;k++)
			{
				if(k>=entry.size()){cout<<"k out of size " << k << endl;}
				//if(attributesPerClass[entry[0]].find(entry[k]) == attributesPerClass[entry[0]].end())
				if(entry[k] < 0 || entry[k]>=attr_nv)
				{
					cout << "k: " << k << endl;
					cout << entry[k] << endl;
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
		zip_all_count();
		cout << "initialize classifier complete"<<endl;
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
		cout << endl << "setting attribute in set_zip" << endl;
		for(int i = 0 ; i < num_C; i++){
			for(int j = 0 ; j < attr_nv; j++){
				//attributePerClass[i][j] = zip_ca[ind];
				attributePerClass[i][j] = a[ind];
				cout << attributePerClass[i][j] << " ";
				++ind;
			}
			cout << endl;
		}
		cout << endl << "finish setting attribute in zip"<<endl;
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
		cout << "calculating probability "<< endl;
		int total_classes = 0;
		for(int i = 0 ; i < num_C; i++){
			total_classes += classes[i];
		}
		cout <<"printing attribute probability"<< endl;
		for(int i=0; i<num_C; i++){
			classes_prob[i] = classes[i]/double(total_classes);
			for(int j=0; j < attr_nv; j++){
				attr_prob[i][j] = attributePerClass[i][j]/double(classes[i]);
				cout << attr_prob[i][j] << " ";
			}
			cout << endl;
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
void populateData(vector<vector<int>>& data, map<string, int>& classmap,
	vector<map<string, int>> attrimap,
	string c, vector<string> attrs)
{
	vector<int> apair;// = { classmap[c],attrimap[a1], attrimap[a2] };
	apair.push_back(classmap[c]);
	int attr_values_num = 0;
	for (int i = 0; i < attrs.size(); i++) {
		apair.push_back(attr_values_num + attrimap[i][attrs[i]]);
		attr_values_num += attrimap[i].size();
	}
	vector<vector<int>> newarr(1, apair);
	data.insert(data.end(), newarr.begin(), newarr.end());
//	cout << "data size: " << data.size() << endl;
	return;

}


void read_data(vector<vector<int>>& data, string filename, map<string, int>& classmap,
	vector<map<string, int>>& attrimap) {
	ifstream in(filename);
	if (!in)
	{
		cerr << "Cannot open the File : " << filename << std::endl;
		return;
	}
	string c, line, attr;
	cout << "reading files" << endl;
	while (getline(in, line)) {
		istringstream iss(line);
		iss >> c;
		vector<string> attrs;
		bool missing_value = 0;
		while (iss >> attr) {
			if(attr=="?") missing_value = 1;
			attrs.push_back(attr);
		}
		if (missing_value) continue;
		populateData(data, classmap, attrimap, c, attrs);
	}
	cout << "data construction finished" << endl;

}
	
/*
edoid read_data(vector<vector<int>>& data, string filename,map<string,int> & classmap, 
	map<string,int> & attrimap) {
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
*/
int read_classmap(map<string, int>& classmap, string filename) {
	ifstream in(filename);
	if (!in)
	{
		cerr << "Cannot open class map file : " << filename << std::endl;
		return -1;
	}
	cout << "reading class map" << endl;
	string cls;
	int cnt = 0;
	while (in >> cls) {
		classmap[cls] = cnt;
		cnt++;
	}
	return cnt;
}

int read_attrmap(vector<map<string, int>>& attrmap, string filename) {
	ifstream in(filename);
	if (!in)
	{
		cerr << "Cannot open attribute map file : " << filename << std::endl;
		return -1;
	}
	cout << "reading attribute map" << endl;
	string att;
	int num_attr = 0;
	string line;
	int attr_values_cnt = 0;
	while (getline(in, line)) {
		int cnt = 0;
		map<string, int> a_map;
		istringstream iss(line);
		while (iss >> att) {
			a_map[att] = cnt;
			++cnt;
			++attr_values_cnt;
		}
		attrmap.push_back(a_map);
	}
	return attr_values_cnt;
}
/*
int main() {
	// prepare a training dataset with 2 attributes and 3 classes
	map<string, int> classmap;
	vector<vector<int>> data;
	vector<map<string, int>> attr_maps;
	//string attr_file = "L:\\learning\\academy\\TUeM1\\Q4\\Seminar\\just_nb\\nb\\nb\\data\\balance_scale\\balance-scale.attrs";
	//string cls_file = "L:\\learning\\academy\\TUeM1\\Q4\\Seminar\\just_nb\\nb\\nb\\data\\balance_scale\\balance-scale.class";
	//string d_file = "L:\\learning\\academy\\TUeM1\\Q4\\Seminar\\just_nb\\nb\\nb\\data\\balance_scale\\balance-scale-client.data";
	string attr_file = "L:\\learning\\academy\\TUeM1\\Q4\\Seminar\\just_nb\\nb\\nb\\data\\breast_cancer\\breast-cancer.attrs";
	string cls_file = "L:\\learning\\academy\\TUeM1\\Q4\\Seminar\\just_nb\\nb\\nb\\data\\breast_cancer\\breat-cancer.class";
	string d_file = "L:\\learning\\academy\\TUeM1\\Q4\\Seminar\\just_nb\\nb\\nb\\data\\breast_cancer\\breast-cancer-client.data";
	int attr_values_cnt = read_attrmap(attr_maps,attr_file);
	int C = read_classmap(classmap, cls_file);
	read_data(data, d_file, classmap, attr_maps);

	NaiveBayesClassifer mymodel(data, attr_maps.size(),classmap.size(), attr_values_cnt);

	return 0;
	/*
	//read_classmap("data/")
	//map<string, int> classmap = {{"apple", 0}, {"pineapple", 1}, {"cherry", 2}};
	//map<string, int> attrimap =
	// color
	//{{"red", 0}, {"green", 1}, {"yellow", 2},
	// shape
	//{"round", 3}, {"oval", 4}, {"heart", 5}};
	vector<vector<int>> data;
	//read_data(data, "train_s.txt",classmap,attrimap);
	
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
