/*
 * KeeLOQ.h
 *
 *  Created on: Jan 6, 2024
 *      Author: Mr. Han
 */

#ifndef KEELOQ_H_
#define KEELOQ_H_

#include <stdint.h>

#define NLF_LOOKUP_CONSTANT 0x3A5C742E

char nlf(int d);
void keeloq_encrypt(long long *key, uint32_t *plaintext, uint32_t *ciphertext, int nrounds);
void keeloq_decrypt(long long *key, uint32_t *plaintext, uint32_t *ciphertext, int nrounds);


#endif /* KEELOQ_H_ */
