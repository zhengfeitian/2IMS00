#include "aby/ABY/src/abycore/circuit/booleancircuits.h"
#include "aby/ABY/src/abycore/circuit/arithmeticcircuits.h"
#include "aby/ABY/src/abycore/circuit/circuit.h"
#include "aby/ABY/src/abycore/aby/abyparty.h"
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

int main(){
	cout << "hello" <<endl;
	ABYParty* party = new ABYParty(role, address, port, seclvl, bitlen, nthreads,
						mt_alg);
	std::vector<Sharing*>& sharings = party->GetSharings();
	Circuit* circ = sharings[sharing]->GetCircuitBuildRoutine();
	share *s_alice_money, *s_bob_money, *s_out;
	uint32_t alice_money, bob_money, output;
	srand(time(NULL));
	alice_money = rand();
	bob_money = rand();

	if(role == SERVER) {
		s_alice_money = circ->PutDummyINGate(bitlen);
		s_bob_money = circ->PutINGate(bob_money, bitlen, SERVER);
	} else { //role == CLIENT
		s_alice_money = circ->PutINGate(alice_money, bitlen, CLIENT);
		s_bob_money = circ->PutDummyINGate(bitlen);
	}

	s_out = BuildMillionaireProbCircuit(s_alice_money, s_bob_money,
						(BooleanCircuit*) circ);
	s_out = circ->PutOUTGate(s_out, ALL);
	party->ExecCircuit();

	output = s_out->get_clear_value<uint32_t>();

	std::cout << "Testing Millionaire's Problem in " << get_sharing_name(sharing)
						<< " sharing: " << std::endl;
	std::cout << "\nAlice Money:\t" << alice_money;
	std::cout << "\nBob Money:\t" << bob_money;
	std::cout << "\nCircuit Result:\t" << (output ? ALICE : BOB);
	std::cout << "\nVerify Result: \t" << ((alice_money > bob_money) ? ALICE : BOB) << endl;

	delete party;
	return 0;

}

int32_t test_millionaire_prob_simple_circuit ( e_role role ) {
// Setup parameters
      string address = " 127.0.0.1 ";
uint16_t port = 6677;
seclvl seclvl = get_sec_lvl (128) ;
e_sharing sharing = S_YAO ;
uint32_t bitlen = 32;
uint32_t nthreads = 1;
ABYParty * party = new ABYParty ( role , ( char *) address . c_str () ,
      	port , seclvl , bitlen , nthreads ) ;
 vector < Sharing * >& sharings = party - > GetSharings () ;
 Circuit * circ = sharings [ sharing ] - > GetCircuitBuildRoutine () ;

      // Plaintext values for testing and their bit length
      uint32_t alice_money = 5;
 uint32_t bob_money = 7;
 uint32_t money_bitlen = 3;

      // Input shares
      share * s_alice_money = circ - > PutINGate ( alice_money ,
      		money_bitlen , CLIENT ) ;
 share * s_bob_money = circ - > PutINGate ( bob_money , money_bitlen ,
      	SERVER ) ;

      // Greater - than operation
      share * s_out = circ - > PutGTGate ( s_alice_money , s_bob_money ) ;

     // Output share
      s_out = circ - > PutOUTGate ( s_out , ALL ) ;

 // Execute secure computation protocol
       party - > ExecCircuit () ;

       // Get plain text output
       uint32_t output = s_out - > get_clear_value < uint32_t >() ;

       // Verification
      cout << " Testing Millionaire ’s Problem in " <<
      get_sharing_name ( sharing ) << " sharing : " << endl ;
	printf ("\ nAlice Money :\t %d", alice_money ) ;
	printf ("\ nBob Money :\t %d", bob_money ) ;
	printf ("\ nCircuit Result :\t %s" ,( output ? " Alice " : "Bob") ) ;
	printf ("\ nVerify Result : \t %s\n" ,(( alice_money > bob_money ) ?
      		" Alice " : "Bob") ) ;

	delete party ;
	return 0;
 }

