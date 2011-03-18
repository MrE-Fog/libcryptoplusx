/**
 * \file pkey.cpp
 * \author Julien Kauffmann <julien.kauffmann@freelan.org>
 * \brief A PKEY sample file.
 */

#include <cryptoplus/pkey/pkey.hpp>
#include <cryptoplus/hash/message_digest_context.hpp>
#include <cryptoplus/error/error_strings.hpp>

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
	cryptoplus::error::error_strings_initializer error_strings_initializer;
	cryptoplus::cipher::cipher_initializer cipher_initializer;

	std::cout << "PKEY sample" << std::endl;
	std::cout << "===========" << std::endl;
	std::cout << std::endl;

	const std::string private_key_filename = "private_key.pem";
	const std::string certificate_public_key_filename = "certificate_public_key.pem";

	boost::shared_ptr<FILE> private_key_file(fopen(private_key_filename.c_str(), "w"), fclose);
	boost::shared_ptr<FILE> certificate_public_key_file(fopen(certificate_public_key_filename.c_str(), "w"), fclose);

	if (!private_key_file)
	{
		std::cerr << "Unable to open \"" << private_key_filename << "\" for writing." << std::endl;

		return EXIT_FAILURE;
	}

	if (!certificate_public_key_file)
	{
		std::cerr << "Unable to open \"" << certificate_public_key_filename << "\" for writing." << std::endl;

		return EXIT_FAILURE;
	}

	try
	{
		std::cout << "Generating DSA key. This can take some time..." << std::endl;

		cryptoplus::pkey::dsa_key dsa_key = cryptoplus::pkey::dsa_key::generate_private_key(1024, NULL, 0, NULL, NULL);

		std::cout << "Done." << std::endl;

		cryptoplus::pkey::pkey pkey;

		pkey.set_dsa_key(dsa_key);

		std::cout << "Checking that the type is correct: " << (pkey.is_dsa() ? "OK" : "FAILURE") << std::endl;

		pkey.write_private_key_pkcs8(private_key_file.get(), cryptoplus::cipher::cipher_algorithm("AES256"), pem_passphrase_callback);

		std::cout << "Private key written succesfully to \"" << private_key_filename << "\"." << std::endl;

		pkey.write_certificate_public_key(certificate_public_key_file.get());

		std::cout << "Certificate public key written succesfully to \"" << certificate_public_key_filename << "\"." << std::endl;
	}
	catch (std::exception& ex)
	{
		std::cerr << "Exception: " << ex.what() << std::endl;

		return EXIT_FAILURE;
	}

	certificate_public_key_file.reset();
	private_key_file.reset(fopen(private_key_filename.c_str(), "r"), fclose);

	if (!private_key_file)
	{
		std::cerr << "Unable to open \"" << private_key_filename << "\" for reading." << std::endl;

		return EXIT_FAILURE;
	}

	try
	{
		std::cout << "Trying to read back the private key from \"" << private_key_filename << "\"..." << std::endl;

		cryptoplus::pkey::pkey pkey = cryptoplus::pkey::pkey::from_private_key(private_key_file.get(), pem_passphrase_callback);

		std::cout << "Done." << std::endl;
	}
	catch (std::exception& ex)
	{
		std::cerr << "Exception: " << ex.what() << std::endl;

		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}