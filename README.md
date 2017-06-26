# nfacctd-snmp-check
This is a short C++ programm that should be used with keepalived MISC_CHECK check
to confirm that nfacctd service is running on specific host.

## What's going on here?
Check process has tree components:
* Local check `proc_check` - On the nfacct server
* Extended snmpd OIDS (example in `snmpd.conf`) - On the nfacct server
* `nfacctd_check` - On the loadbalancer (KeepaliveD)

Local check (`proc_check`) runs the simple bash command that counts number of proccesses that match a given argument (`nfacctd`).
Output from this check is bound via SNMPD config to a custom (extended) OID that is polled by `nfacctd_check`.
If the number of proccesses is zero, nfacctd server should be considered offline by the loadbalancer (KeepaliveD).
This is done by KeepaliveD `MISC_CHECK` check. Example part of the `keepalived.conf` config for a real server using this check:
```
real_server <ip_address> <port> {
    weight 1
    MISC_CHECK {
      misc_path "/usr/local/sbin/nfacctd_check <nfacctd_server_hostname>"
    }
  }
}
```
Since `nfacctd_check` exits with error if there's no running nfacctd proccess on `<nfacctd_server_hostname>`
(or if there is some kind of SNMP error), the check will fail in those cases and exclude the real server from loadbalancing.
