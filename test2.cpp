#include "aby/ABY/src/abycore/circuit/booleancircuits.h"
#include "aby/ABY/src/abycore/circuit/arithmeticcircuits.h"
#include "aby/ABY/src/abycore/circuit/circuit.h"
#include "aby/ABY/src/abycore/aby/abyparty.h"
#include "aby/ABY/src/abycore/sharing/sharing.h"
#include <ENCRYPTO_utils/crypto/crypto.h>
#include <ENCRYPTO_utils/parse_options.h>

#include <math.h>
#include <cassert>
#include<iostream>

using namespace std;

int32_t read_test_options(int32_t* argcp, char*** argvp, e_role* role,
				uint32_t* bitlen, uint32_t* nvals, uint32_t* secparam, std::string* address,
						uint16_t* port, int32_t* test_op) {

	uint32_t int_role = 0, int_port = 0;

	parsing_ctx options[] =
		{ { (void*) &int_role, T_NUM, "r", "Role: 0/1", true, false }, {
		(void*) nvals, T_NUM, "n",
		"Number of parallel operation elements", false, false }, {
													(void*) bitlen, T_NUM, "b", "Bit-length, default 32", false,
													false }, { (void*) secparam, T_NUM, "s",
													"Symmetric Security Bits, default: 128", false, false }, {
													(void*) address, T_STR, "a",
													"IP-address, default: localhost", false, false }, {
													(void*) &int_port, T_NUM, "p", "Port, default: 7766", false,
													false }, { (void*) test_op, T_NUM, "t",
													"Single test (leave out for all operations), default: off",
													false, false } };

		if (!parse_options(argcp, argvp, options,
										sizeof(options) / sizeof(parsing_ctx))) {
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


int32_t test_millionaire_prob_simple_circuit ( e_role role ) {
// Setup parameters
      string address = " 127.0.0.1 ";
	uint16_t port = 12421;
	seclvl seclvl = get_sec_lvl (128) ;
	e_sharing sharing = S_YAO ;
	uint32_t bitlen = 32;
	uint32_t nthreads = 1;
	ABYParty * party = new ABYParty ( role , ( char *) address . c_str () ,
      	port , seclvl , bitlen , nthreads ) ;
	vector<Sharing*>& sharings = party -> GetSharings () ;
	Circuit * circ = sharings[sharing]->GetCircuitBuildRoutine() ;

      // Plaintext values for testing and their bit length
	uint32_t alice_money = 5;
	uint32_t bob_money = 7;
	uint32_t money_bitlen = 3;

      // Input shares
      share * s_alice_money = circ -> PutINGate ( alice_money ,
      		money_bitlen , CLIENT ) ;
 share * s_bob_money = circ -> PutINGate ( bob_money , money_bitlen ,
      	SERVER ) ;

      // Greater - than operation
      share * s_out = circ -> PutGTGate ( s_alice_money , s_bob_money ) ;

     // Output share
      s_out = circ -> PutOUTGate ( s_out , ALL ) ;

 // Execute secure computation protocol
       party -> ExecCircuit () ;

       // Get plain text output
       uint32_t output = s_out -> get_clear_value < uint32_t >() ;

       // Verification
      cout << " Testing Millionaire ’s Problem in " <<
      get_sharing_name ( sharing ) << " sharing : " << endl ;
	printf ("\nAlice Money :\t %d", alice_money ) ;
	printf ("\nBob Money :\t %d", bob_money ) ;
	printf ("\nCircuit Result :\t %s" ,( output ? " Alice " : "Bob") ) ;
	printf ("\nVerify Result : \t %s\n" ,(( alice_money > bob_money ) ?
      		" Alice " : "Bob") ) ;

	delete party ;
	return 0;
 }

int main(int argc, char** argv) {

	e_role role;
	uint32_t bitlen = 32, nvals = 31, secparam = 128, nthreads = 1;
	uint16_t port = 7766;
	std::string address = "127.0.0.1";
	int32_t test_op = -1;
	e_mt_gen_alg mt_alg = MT_OT;

	read_test_options(&argc, &argv, &role, &bitlen, &nvals, &secparam, &address,
						&port, &test_op);


	//test_millionaire_prob_circuit(role, address, port, seclvl, 32,
	//				nthreads, mt_alg, S_YAO);
	test_millionaire_prob_simple_circuit(role);
										//
	return 0;
}
										//
