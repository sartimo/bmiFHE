#include "seal/seal.h"
#include <cmath>
#include <vector>
#include <iostream>

using namespace std;
using namespace seal;

int main()
{
    // Create encryption parameters
    EncryptionParameters parms(scheme_type::ckks);
    size_t poly_modulus_degree = 8192;
    parms.set_poly_modulus_degree(poly_modulus_degree);
    parms.set_coeff_modulus(CoeffModulus::Create(poly_modulus_degree, { 60, 40, 40, 60 }));

    // Set scale
    double scale = pow(2.0, 40);

    // Create SEALContext
    SEALContext context(parms);
    cout << endl;

    // Generate keys
    KeyGenerator keygen(context);
    auto secret_key = keygen.secret_key();
    PublicKey public_key;
    keygen.create_public_key(public_key);
    RelinKeys relin_keys;
    keygen.create_relin_keys(relin_keys);
    GaloisKeys gal_keys;
    keygen.create_galois_keys(gal_keys);
    Encryptor encryptor(context, public_key);
    Evaluator evaluator(context);
    Decryptor decryptor(context, secret_key);

    // Create CKKSEncoder
    CKKSEncoder encoder(context);
    size_t slot_count = encoder.slot_count();

    // Take user inputs for weight and height
    double weight, height;
    cout << "Enter weight (kg): ";
    cin >> weight;
    cout << "Enter height (m): ";
    cin >> height;

    // Prepare inputs
    vector<double> weight_input = { weight };
    vector<double> height_input = { height };

    // Encode and encrypt the inputs
    Plaintext plain_weight, plain_height;
    encoder.encode(weight_input, scale, plain_weight);
    encoder.encode(height_input, scale, plain_height);

    Ciphertext encrypted_weight, encrypted_height;
    encryptor.encrypt(plain_weight, encrypted_weight);
    encryptor.encrypt(plain_height, encrypted_height);

    // Compute height^2
    Ciphertext encrypted_height_squared;
    evaluator.square(encrypted_height, encrypted_height_squared);
    evaluator.relinearize_inplace(encrypted_height_squared, relin_keys);
    evaluator.rescale_to_next_inplace(encrypted_height_squared);

    // Compute the multiplicative inverse of height^2
    Plaintext plain_inverse_height_squared;
    vector<double> inverse_height_squared = { 1.0 / (height * height) };
    encoder.encode(inverse_height_squared, scale, plain_inverse_height_squared);

    Ciphertext encrypted_inverse_height_squared;
    encryptor.encrypt(plain_inverse_height_squared, encrypted_inverse_height_squared);

    // Compute BMI: weight / (height^2) is approximated by weight * (1 / height^2)
    Ciphertext encrypted_bmi;
    evaluator.multiply(encrypted_weight, encrypted_inverse_height_squared, encrypted_bmi);
    evaluator.rescale_to_next_inplace(encrypted_bmi);

    // Decrypt and decode the result
    Plaintext plain_bmi;
    decryptor.decrypt(encrypted_bmi, plain_bmi);
    vector<double> bmi_result;
    encoder.decode(plain_bmi, bmi_result);

    cout << "BMI: " << bmi_result[0] << endl;

    return 0;
}
