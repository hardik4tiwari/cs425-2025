# A2: Iterative and Recursive DNS lookup

## Step to run the code
Open the terminal in the directory having dns_server.py
run the command "python dns_server.py <iterative | recursive> < domain >
## Explaination of todos
### TODO 1 Sending DNS query to server: 
  
    response = dns.query.udp(query, server, timeout=TIMEOUT) 
    return response 

 - We have used dns.query.udp() method from the dnspython library that sends the DNS query over the UDP protocol to the specified server.
 - The timeout parameter sets the maximum time in seconds the client waits for the server to respond before giving up. If the server doesn't respond within 2 seconds, the function will raise an exception.

### TODO 2 Resolving Hostnames to IP Addresses:

    for ns_name in ns_names:
        try:
            answer = dns.resolver.resolve(ns_name, "A")
            for rdata in answer:
                ns_ips.append(rdata.address)
        except Exception as e:
            print(f"[ERROR] Failed to resolve {ns_name}: {e}")
                
- The loop iterates through each nameserver hostname in the ns_names list.
- The dns.resolver.resolve() method performs a DNS query to resolve the hostname to an A record (IPv4 address).
- If the hostname resolves successfully, the IP address is appended to the list ns_ips.
- If the resolution fails (due to DNS errors, timeouts, or nonexistent records), an exception is caught, and an error message is printed.

### TODO 3 Move to the next resolution stage:
    if stage == "ROOT":
        stage = "TLD"
    elif stage == "TLD":
        stage = "AUTH"

- We use an if-elif structure to update the stage variable based on its current value.

- If the current stage is "ROOT", we move to the "TLD" (Top-Level Domain) stage. This happens after querying the root servers, which typically return referrals to TLD servers.

- If the current stage is "TLD", we move to the "AUTH" (Authoritative) stage. This occurs after querying TLD servers, which usually provide referrals to authoritative nameservers for the specific domain.

- We don't need to change the stage if it's already "AUTH" because authoritative servers are the final step in the resolution process. They either provide the answer or the resolution fails.


### TODO4 recursive DNS resolution using the system's default resolver

    answer = dns.resolver.resolve(domain, "NS") 

- dns.resolver.resolve() is a function from the dnspython library that performs DNS resolution.

- It takes two main arguments :
    -  domain: The domain name we want to resolve.
    - "NS": The record type we're looking for (in this case, Name Server records).

- The function uses the system's default DNS resolver to perform a recursive lookup. This means, it starts by querying the root servers. Then it follows referrals to the appropriate Top-Level Domain (TLD) servers. Finally, it queries the authoritative nameservers for the specific domain.

- The resolver handles all these steps internally, simplifying the process for the user.

- The result is stored in the answer variable, which contains an iterable of DNS record data (RData) objects.


