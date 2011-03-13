/**
 * \file dh.cpp
 * \author Julien Kauffmann <julien.kauffmann@freelan.org>
 * \brief A DH sample file.
 */

#include <cryptopen/pkey/dh.hpp>
#include <cryptopen/hash/message_digest_context.hpp>
#include <cryptopen/error/error_strings.hpp>

#include <boost/shared_ptr.hpp>

#include <iostream>
#include <string>
#include <cstdio>

namespace
{
	int pem_passphrase_callback(char* buf, int buf_len, int rwflag, void*)
	{
		std::cout << "Passphrase (max: " << buf_len << " characters): " << std::flush;
		std::string passphrase;
		std::getline(std::cin, passphrase);

		if (passphrase.empty())
		{
			std::cerr << "Passphrase cannot be empty." << std::endl;
			return 0;
		}

		if (passphrase.size() > static_cast<size_t>(buf_len))
		{
			std::cerr << "Passphrase cannot exceed " << buf_len << " characters." << std::endl;
			return 0;
		}

		if (rwflag != 0)
		{
			std::cout << "Confirm: " << std::flush;
			std::string passphrase_confirmation;
			std::getline(std::cin, passphrase_confirmation);

			if (passphrase_confirmation != passphrase)
			{
				std::cerr << "The two passphrases do not match !" << std::endl;
				return 0;
			}
		}

		std::copy(passphrase.begin(), passphrase.end(), buf);
		return passphrase.size();
	}
}

int main()
{
	cryptopen::error::error_strings_initializer error_strings_initializer;
	cryptopen::cipher::cipher_initializer cipher_initializer;

	std::cout << "DH sample" << std::endl;
	std::cout << "=========" << std::endl;
	std::cout << std::endl;

	const int bits = 1024;
	const int generator = 2;

	std::cout << "Using DH keys of " << bits << " bits." << std::endl;

	const std::string parameters_filename = "parameters.pem";

	boost::shared_ptr<FILE> parameters_file(fopen(parameters_filename.c_str(), "w"), fclose);

	if (!parameters_file)
	{
		std::cerr << "Unable to open \"" << parameters_filename << "\" for writing." << std::endl;

		return EXIT_FAILURE;
	}

	try
	{
		std::cout << "Generating DH parameters. This can take some time..." << std::endl;

		cryptopen::pkey::dh dh = cryptopen::pkey::dh::generate_parameters(bits, generator);

		int codes = 0;

		dh.check(codes);

		if (codes != 0)
		{
			std::cerr << "Generation failed." << std::endl;

			if (codes & DH_CHECK_P_NOT_SAFE_PRIME)
			{
				std::cerr << "p is not a safe prime." << std::endl;
			}
			if (codes & DH_NOT_SUITABLE_GENERATOR)
			{
				std::cerr << "g is not a suitable generator." << std::endl;
			}

			if (codes & DH_UNABLE_TO_CHECK_GENERATOR)
			{
				std::cerr << "g is not a correct generator. Must be either 2 or 5." << std::endl;
			}

			return EXIT_FAILURE;
		}

		dh.write_parameters(parameters_file.get());

		std::cout << "DH parameters written succesfully to \"" << parameters_filename << "\"." << std::endl;
		std::cout << "Done." << std::endl;

		std::cout << "Generating DH key..." << std::endl;

		dh.generate_key();

		std::cout << "Done." << std::endl;

		parameters_file.reset(fopen(parameters_filename.c_str(), "r"), fclose);

		if (!parameters_file)
		{
			std::cerr << "Unable to open \"" << parameters_filename << "\" for reading." << std::endl;

			return EXIT_FAILURE;
		}

		std::cout << "Trying to read back the DH parameters from \"" << parameters_filename << "\"..." << std::endl;

		cryptopen::pkey::dh dh2 = cryptopen::pkey::dh::from_parameters(parameters_file.get(), pem_passphrase_callback);

		std::cout << "Done." << std::endl;

		std::cout << "Generating DH key..." << std::endl;

		dh2.generate_key();

		std::cout << "Done." << std::endl;

		std::cout << "Computing key A..." << std::endl;

		std::vector<unsigned char> key_a = dh.compute_key<unsigned char>(dh2.public_key());
		
		std::cout << "Done." << std::endl;

		std::cout << "Computing key B..." << std::endl;

		std::vector<unsigned char> key_b = dh2.compute_key<unsigned char>(dh.public_key());
		
		std::cout << "Done." << std::endl;

		std::cout << "Comparing key A and key B: " << ((key_a == key_b) ? "IDENTICAL" : "DIFFERENT") << std::endl;
	}
	catch (std::exception& ex)
	{
		std::cerr << "Exception: " << ex.what() << std::endl;

		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}