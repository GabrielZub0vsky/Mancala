/*
 * clientdemo.h
 *
 *  Created on: Apr 23, 2025
 *      Author: HomeUser
 */

#ifndef MANCALACLIENT_H_
#define MANCALACLIENT_H_


void init(struct Client* cl);

void doConnect(struct Client* cl, const char* serverAddr);

void doRead(struct Client* cl);


#endif /* MANCALACLIENT_H_ */
