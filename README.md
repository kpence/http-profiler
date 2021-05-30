# httprofile

NOTE: This was originally a project for a job application assignment, but I like this project so I'm reuploading it as a public repo. I deleted the old commit history because it reveals private information.

## Prerequisites

Install pkg-config.
Install OpenSSL.

## How to Build

```
make clean all
```

## How to Use

```
./httprofile
./httprofile --help
```

Use `--url <url>` to specify URL that you want to run the program on.<br>
Use `--check-robots-txt` if you want to check if the site  has a `robots.txt` before running.<br>
Use `--check-robots-txt` if you want to check if the site  has a `robots.txt` before running.<br>
Use `--profile <Number of requests>` to run get statistics from running multiple requests on the domain.<br>
Run the program without `--profile` in order to fetch the page from the domain and print it out.<br>

## Traces

## REDACTED.dev

For fun I ran a profile on my site.

```
URL: https://REDACTED.dev
          Parsing URL... host REDACTED.dev, port 443
        * Connecting on page...  done in 0.256 ms
          SSL connection using ECDHE-RSA-AES256-GCM-SHA384

--------------------------------------
# of Requests: 50
Slowest Time: 0.163 ms
Fastest Time: 0.007 ms
Mean Time: 0.01712 ms
Median Time: 0.011 ms
Smallest Response (Size in Bytes): 1907
Largest Response (Size in Bytes): 1907
Percentage of requests that succeeded: 100%
Number of Failed Request with Status Code: 3xx = 0, 4xx = 0, other = 0
```


### Cloudflare

```
URL: https://www.cloudflare.com
          Parsing URL... host www.cloudflare.com, port 443
        * Connecting on page...  done in 0.515 ms
          SSL connection using ECDHE-ECDSA-CHACHA20-POLY1305

--------------------------------------
# of Requests: 500
Slowest Time: 1.919 ms
Fastest Time: 0.006 ms
Mean Time: 0.013288 ms
Median Time: 0.008 ms
Smallest Response (Size in Bytes): 102569
Largest Response (Size in Bytes): 102569
Percentage of requests that succeeded: 100%
Number of Failed Request with Status Code: 3xx = 0, 4xx = 0, other = 0
```

Cloudflare's mean and median times were lower than for my digitalocean site (and much more bytes being downloaded). The slowest time was long, but that seems to be an outlier, it may have to do with my program loading dynamic memory at the start, or maybe it has to do with the network needing to cache after this initial request.

### REDACTED.edu

```
URL: https://www.REDACTED.edu
          Parsing URL... host www.REDACTED.edu, port 443
        * Connecting on page...  done in 0.223 ms
          SSL connection using ECDHE-RSA-AES256-GCM-SHA384

--------------------------------------
# of Requests: 500
Slowest Time: 0.683 ms
Fastest Time: 0.005 ms
Mean Time: 0.009322 ms
Median Time: 0.007 ms
Smallest Response (Size in Bytes): 70792
Largest Response (Size in Bytes): 70792
Percentage of requests that succeeded: 100%
Number of Failed Request with Status Code: 3xx = 0, 4xx = 0, other = 0
```

REDACTED University's website's mean time was fairly fast. Maybe because I live next to their servers.


### Google

```
$ ./httprofile -p 500 -u https://google.com

URL: https://google.com
          Parsing URL... host google.com, port 443
        * Connecting on page...  done in 0.304 ms
          SSL connection using ECDHE-RSA-CHACHA20-POLY1305

--------------------------------------
# of Requests: 500
Slowest Time: 0.292 ms
Fastest Time: 0.005 ms
Mean Time: 0.012434 ms
Median Time: 0.01 ms
Smallest Response (Size in Bytes): 760
Largest Response (Size in Bytes): 760
Percentage of requests that succeeded: 0%
Number of Failed Request with Status Code: 3xx = 500, 4xx = 0, other = 0
```

Above, you can see that all the requests were receiving `3xx` status codes. To find where the site wants you to redirect, we can remove `--profile` to print the response.

```
$ ./httprofile -u https://google.com
URL: https://google.com
          Parsing URL... host google.com, port 443
        * Connecting on page...  done in 0.218 ms
          SSL connection using ECDHE-RSA-CHACHA20-POLY1305
          Loading... done in 0.187 ms with 760 bytes
          Verifying header... status code 301

--------------------------------------
HTTP/1.0 301 Moved Permanently
Location: https://www.google.com/
Content-Type: text/html; charset=UTF-8
Date: Mon, 19 Oct 2020 04:48:23 GMT
Expires: Wed, 18 Nov 2020 04:48:23 GMT
Cache-Control: public, max-age=2592000
Server: gws
Content-Length: 220
X-XSS-Protection: 0
X-Frame-Options: SAMEORIGIN
Alt-Svc: h3-Q050=":443"; ma=2592000,h3-29=":443"; ma=2592000,h3-27=":443"; ma=2592000,h3-T051=":443"; ma=2592000,h3-T050=":443"; ma=2592000,h3-Q046=":443"; ma=2592000,h3-Q043=":443"; ma=2592000,quic=":443"; ma=2592000; v="46,43"

<HTML><HEAD><meta http-equiv="content-type" content="text/html;charset=utf-8">
<TITLE>301 Moved</TITLE></HEAD><BODY>
<H1>301 Moved</H1>
The document has moved
<A HREF="https://www.google.com/">here</A>.
</BODY></HTML>
```

As you can see it is asking to redirect to `www.google.com`.

```
$ ./httprofile -p 600 -u https://www.google.com

URL: https://www.google.com
          Parsing URL... host www.google.com, port 443
        * Connecting on page...  done in 0.224 ms
          SSL connection using ECDHE-RSA-CHACHA20-POLY1305

--------------------------------------
# of Requests: 500
Slowest Time: 0.564 ms
Fastest Time: 0.006 ms
Mean Time: 0.00909 ms
Median Time: 0.007 ms
Smallest Response (Size in Bytes): 13760
Largest Response (Size in Bytes): 13760
Percentage of requests that succeeded: 100%
Number of Failed Request with Status Code: 3xx = 0, 4xx = 0, other = 0
```

### Big request

For fun I sent a lot of requests to https://www.google.com:

```
URL: https://www.google.com
          Parsing URL... host www.google.com, port 443
        * Connecting on page...  done in 0.203 ms

--------------------------------------
# of Requests: 10000
Slowest Time: 2.687 ms
Fastest Time: 0.024 ms
Mean Time: 0.0451914 ms
Median Time: 0.03 ms
Smallest Response (Size in Bytes): 14493
Largest Response (Size in Bytes): 15362
Percentage of requests that succeeded: 99.98%
Number of Failed Request with Status Code: 3xx = 0, 4xx = 0, other = 2
```

So, I noticed that my program's connection would cut off if I try to do very large number of requests, and then my program would receive a SIGPIPE. So what I do is reopen the connection when this happens, however I still lose some requests because I don't receive the SIGPIPE until after I've already sent some requests. This is why my success rate isn't 100% for 10000 requests to google. So far I think the only other way of handling this would be to use timeouts.
