#include <stdlib.h>
#include <net-snmp/net-snmp-config.h>
#include <net-snmp/net-snmp-includes.h>

/*
 *  cc -I/usr/local/include -g   -c main.cpp -o main.o
 *  cc -o nfacctd_check main.o -lnetsnmp -lstdc++
 *
 * nfacctd-snmp-check
 * To be used as MISC_CHECK with keepalived
 * Poll extended snmpd for number of running nfacctd processes
 * If there is a running nfacctd proccess exit without errors
 *
 * Copyright (C) 2015  Pavle Obradovic (pajaja)
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */


using namespace std;

int main(int argc, char *argv[]) {

  /*
   * If argument is not provided exit with general error
   */
  if(argc <= 1){
	return 1;
  }


  /*
   * Hostname of the server running the service
   */
  const char* hostname = argv[1];
  /* 
   * SNMP v2 community string (RO)
   */
  const char* community = "public";
  /*
   * Extended (custom) SNMP OID to poll
   * In this case it's NET-SNMP-EXTEND-MIB::nsExtendOutputFull.7."nfacctd"
   * Should return number of processes that match given string ("nfacctd")
   */
  const char* nfacctd_oid = "1.3.6.1.4.1.8072.1.3.2.3.1.2.7.110.102.97.99.99.116.100";


  struct snmp_pdu *s_pdu;
  struct snmp_pdu *s_resp;

  oid oid_arr[MAX_OID_LEN];
  size_t oid_arr_len = MAX_OID_LEN;

  int status;

  struct snmp_session session, *sessionptr;
  init_snmp("snmpapp");
  snmp_sess_init( &session );

  /* Setup SNMP session */
  session.peername = (char*) hostname;
  session.timeout = 1;
  session.version = SNMP_VERSION_2c;
  session.community = (unsigned char*) community;
  session.community_len = strlen(community);

  sessionptr = snmp_open(&session);
  if (!sessionptr) {
	return 1;
  }

  /* SNMP get request */
  s_pdu = snmp_pdu_create(SNMP_MSG_GET);
  read_objid(nfacctd_oid, oid_arr, &oid_arr_len);
  snmp_add_null_var(s_pdu, oid_arr, oid_arr_len);
  status = snmp_synch_response(sessionptr, s_pdu, &s_resp);
  /* If SNMP request is successfull check SNMP response value */
  if (status == STAT_SUCCESS && s_resp->errstat == SNMP_ERR_NOERROR) {
	if(s_resp->variables->val.string){
          /* Convert string value to integer */
	  int proc_num = atoi((const char*)s_resp->variables->val.string);
          /* If the number of running processes is not 0, exit without errors */
	  if(proc_num){
		return 0;
	  }
	}
  }
  /* Default - Exit with general error */
  return 1;
}

