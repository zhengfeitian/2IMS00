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
						uint16_t* port, int32_t* test_op, string* file_name) {

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

	uint32_t nvals = model.num_C;
	uint32_t* alice_class = model.classes;
	uint32_t* bob_class =  model.classes; // place holder
	uint32_t count_bitlen = 32;

	share* sa_classes = circ -> PutSIMDINGate(model.num_C, alice_class,
			count_bitlen, CLIENT);
	share* sb_classes = circ -> PutSIMDINGate(model.num_C, bob_class,
			count_bitlen, SERVER);
	share* s_cls = circ -> PutADDGate(sa_classes, sb_classes);
	// s_out = circ -> PutOUTGate(s_out, ALL);

	// uint32_t out_bitlen, out_nvals, *output_cls_cnt;
	// s_out -> get_clear_value_vec(&output_cls_cnt, &out_bitlen, &out_nvals);

	uint32_t** sum_attr_cnt = new uint32_t*[model.num_C];
	share* s_attrs;

//  attribute probability computation
	for(int i = 0 ; i < model.num_C; i++){
		sum_attr_cnt[i] = new uint32_t[model.attr_nv];

		uint32_t nvals = model.attr_nv;
		uint32_t* alice_attr = model.attributePerClass[i];
		uint32_t* bob_attr =  model.attributePerClass[i];
		uint32_t count_bitlen = 32;

		share* sa_attrs = circ -> PutSIMDINGate(nvals, alice_attr,
				count_bitlen, CLIENT);
		cout << "got sa attrs" << endl;
		share* sb_attrs = circ -> PutSIMDINGate(nvals, bob_attr,
				count_bitlen, SERVER);
		cout << "got sb attrs" << endl;
		share* s_out = circ -> PutADDGate(sa_attrs, sb_attrs);
		cout << "add done" << endl;
		// s_out = circ -> PutOUTGate(s_out, ALL);
		if (i == 0)
			s_attrs = s_out;
		else
			s_attrs = circ -> PutCombinerGate(s_attrs, s_out);
	}
	uint32_t out_bitlen, out_nvals;
	share* s_all = circ -> PutCombinerGate(s_cls, c_attrs);
	s_all = circ -> PutOUTGate(s_all, ALL);
	party -> ExecCircuit();

	s_out -> get_clear_value_vec(&sum_attr_cnt, &out_bitlen, &out_nvals);
	cout << "out_bitlen " << out_bitlen << endl;
	cout << " out nvals " << out_nvals << endl;

	

	// s_out -> get_clear_value_vec(&(sum_attr_cnt[i]), &out_bitlen, &out_nvals);


	// model.setClassesCount(output_cls_cnt);
	model.setAttrCount(sum_attr_cnt);

	// vector<Sharing*>& sharings = party -> GetSharings () ;
	// Circuit * circ = sharings[sharing]->GetCircuitBuildRoutine() ;

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

	read_test_options(&argc, &argv, &role, &bitlen, &nvals, &secparam, &address,
						&port, &test_op, &file_name);

	NaiveBayesClassifer model = train_model(file_name);
	//test_millionaire_prob_circuit(role, address, port, seclvl, 32,
	//				nthreads, mt_alg, S_YAO);
	
	
	secure_sum(model,role);
	return 0;
}
