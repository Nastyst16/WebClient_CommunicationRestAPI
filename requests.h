#ifndef _REQUESTS_
#define _REQUESTS_

// computes and returns a GET or DELETE request string (cookies can be NULL if not needed,
// also the token can be NULL if not needed)
char *compute_get_delete_request(char *host, char *url, char *query_params, char **cookies,
                            	int cookies_count, char *token, int getOrDelete);


// computes and returns a POST request string (cookies can be NULL if not needed)
char *compute_post_request(char *host, char *url, char* content_type, char **body_data,
							int body_data_fields_count, char** cookies, int cookies_count, char *token);

#endif
