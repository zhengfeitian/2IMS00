#include "aby/ABY/src/abycore/circuit/booleancircuits.h"
#include "aby/ABY/src/abycore/circuit/arithmeticcircuits.h"
#include "aby/ABY/src/abycore/circuit/circuit.h"
#include "aby/ABY/src/abycore/aby/abyparty.h"
#include "aby/ABY/src/abycore/sharing/sharing.h"
#include <ENCRYPTO_utils/crypto/crypto.h>
#include <ENCRYPTO_utils/parse_options.h>

#include "NaiveBayesClassifier.cpp"

#include <math.h>
#include <cassert>
#include<iostream>
#include <map>
#include <algorithm>
#include <vector>
#include <fstream>
#include <unordered_map>
#include <cstdlib>

using namespace std;





int32_t read_test_options(int32_t* argcp, char*** argvp, e_role* role,
				uint32_t* bitlen, uint32_t* nvals, uint32_t* secparam, string* address,
						uint16_t* port, int32_t* test_op, string* file_name, 
						string* cls_file, string* attr_file) {

	uint32_t int_role = 0, int_port = 0;

	parsing_ctx options[] =
		{ { (void*) &int_role, T_NUM, "r", "Role: 0/1", true, false }, 
		{(void*) nvals, T_NUM, "n",
						"Number of parallel operation elements", false, false }, 
		{(void*) bitlen, T_NUM, "b", "Bit-length, default 32", false, false }, 
		{ (void*) secparam, T_NUM, "s",
						"Symmetric Security Bits, default: 128", false, false }, 
		{(void*) address, T_STR, "a", "IP-address, default: localhost", false, false }, 
		{(void*) &int_port, T_NUM, "p", "Port, default: 7766", false, false }, 
		{ (void*) test_op, T_NUM, "t", "Single test (leave out for all operations), default: off",
						false, false },
		{(void*) file_name, T_STR, "f", "Training data file, default: train_all.txt",
						false,false},
		{(void*) cls_file, T_STR, "cf", "Class data file, default: None",
						false,false},
		{(void*) attr_file, T_STR, "af", "Attribute data file, default: None",
						false,false}};

	if (!parse_options(argcp, argvp, options,	sizeof(options) / sizeof(parsing_ctx))) {
		print_usage(*argvp[0], options, sizeof(options) / sizeof(parsing_ctx));
		std::cout << "Exiting" << std::endl;
		exit(0);
	}

	assert(int_role < 2);
	*role = (e_role) int_role;

	if (int_port != 0) {
		assert(int_port < 1 << (sizeof(uint16_t) * 8));
		*port = (uint16_t) int_port;
	}

								//delete options;
								//
	return 1;
}


NaiveBayesClassifer train_model(string filename, string cls_file, string attr_file){
	unordered_map<string, int> classmap;
	vector<unordered_map<string, int>> attrimap;
	vector<vector<int>> data;
	int C = read_classmap(classmap, cls_file);
	int attr_values_cnt = read_attrmap(attrimap,attr_file);
	read_data(data, filename, classmap, attrimap);
	
	// random_shuffle(data.begin(),data.end());

	// train model
	NaiveBayesClassifer model(data, attrimap.size(), classmap.size(), attr_values_cnt);
	return model;
} 
NaiveBayesClassifer train_model(string filename="train_s.txt"){
	unordered_map<string, int> classmap = {{"apple", 0}, {"pineapple", 1}, {"cherry", 2}};
	unordered_map<string, int> attrimap =
	// color
	{{"red", 0}, {"green", 1}, {"yellow", 2},
	// shape
	{"round", 3}, {"oval", 4}, {"heart", 5}};
	vector<vector<int>> data;
	read_data(data, filename,classmap,attrimap);
	
	random_shuffle(data.begin(),data.end());

	// train model
	NaiveBayesClassifer model(data, 2, classmap.size(), attrimap.size());
	return model;
} 

void secure_sum(NaiveBayesClassifer& model, e_role role){
	string address = "127.0.0.1";
	uint16_t port = 12421;
	seclvl seclvl = get_sec_lvl (128) ;
	e_sharing sharing = S_YAO ;
	uint32_t bitlen = 32;
	uint32_t nthreads = 1;
	
	ABYParty * party = new ABYParty ( role , ( char *) address.c_str() ,
      	port , seclvl , bitlen , nthreads ) ;
	vector<Sharing*>& sharings = party -> GetSharings () ;
	Circuit * circ = sharings[sharing]->GetCircuitBuildRoutine() ;

	uint32_t nvals = model.num_C + model.num_C * model.attr_nv;
	uint32_t* alice_class = model.zip_ca;
	uint32_t* bob_class =  model.zip_ca; // place holder
	uint32_t count_bitlen = 32;

	cout << "nvals " << nvals << endl;

	share* sa_classes = circ -> PutSIMDINGate(nvals, alice_class,
			count_bitlen, CLIENT);
	share* sb_classes = circ -> PutSIMDINGate(nvals, bob_class,
			count_bitlen, SERVER);
	share* s_out = circ -> PutADDGate(sa_classes, sb_classes);

	s_out = circ -> PutOUTGate(s_out, ALL);
	uint32_t out_bitlen, out_nvals, *sum_zip;
	party -> ExecCircuit();
	s_out -> get_clear_value_vec(&sum_zip, &out_bitlen, &out_nvals);
	cout << "out bit len" << out_bitlen << " out nvals " << out_nvals << endl;
	for(int i = 0 ; i < out_nvals; i++){
		cout << sum_zip[i] << " " ;
	}
	cout << endl;
	model.set_zip(sum_zip);

	party -> Reset();
	delete party;

	return;
	
	// sum model.attributesPerClass
}

int main(int argc, char** argv) {

	e_role role;
	uint32_t bitlen = 32, nvals = 31, secparam = 128, nthreads = 1;
	uint16_t port = 11421;
	string address = "127.0.0.1";
	int32_t test_op = -1;
	e_mt_gen_alg mt_alg = MT_OT;
	string file_name = "train_s.txt";
	string attr_file, cls_file;

	read_test_options(&argc, &argv, &role, &bitlen, &nvals, &secparam, &address,
						&port, &test_op, &file_name, &cls_file, &attr_file);

	NaiveBayesClassifer model = train_model(file_name, cls_file, attr_file);
	//test_millionaire_prob_circuit(role, address, port, seclvl, 32,
	//				nthreads, mt_alg, S_YAO);
	
	
	secure_sum(model,role);
	model.calProbability();
	return 0;
}
