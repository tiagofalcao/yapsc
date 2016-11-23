#include "yapsc_tcp.h"
#include <assert.h>
#include <expio.h>
#include <stdbool.h>
#include <string.h>

#include <avahi-client/client.h>
#include <avahi-client/lookup.h>

#include <avahi-common/error.h>
#include <avahi-common/malloc.h>
#include <avahi-common/simple-watch.h>

static AvahiClient *client = NULL;
static AvahiServiceBrowser *sb = NULL;
static AvahiSimplePoll *simple_poll = NULL;

static char *name = NULL;
static char *address = NULL;
static int port = 0;

static void resolve_callback(
    AvahiServiceResolver *r, AVAHI_GCC_UNUSED AvahiIfIndex interface,
    AVAHI_GCC_UNUSED AvahiProtocol protocol, AvahiResolverEvent event,
    const char *n, const char *type, const char *domain, const char *host_name,
    const AvahiAddress *addr, uint16_t p, AvahiStringList *txt,
    AvahiLookupResultFlags flags, AVAHI_GCC_UNUSED void *userdata) {

	assert(r);

	/* Called whenever a service has been resolved successfully or timed out
	 */

	switch (event) {
	case AVAHI_RESOLVER_FAILURE:
		EXPIO_LOG(100, "Failed to resolve service '%s' of type '%s' in "
			       "domain '%s': %s",
			  n, type, domain,
			  avahi_strerror(avahi_client_errno(
			      avahi_service_resolver_get_client(r))));
		break;

	case AVAHI_RESOLVER_FOUND: {
		char a[AVAHI_ADDRESS_STR_MAX], *t;

		avahi_address_snprint(a, sizeof(a), addr);
		t = avahi_string_list_to_string(txt);
		EXPIO_LOG(100, "Service '%s' of type '%s' in domain '%s':\n"
			       "\t%s:%u (%s)\n"
			       "\tTXT=%s\n"
			       "\tcookie is %u\n"
			       "\tis_local: %i\n"
			       "\tour_own: %i\n"
			       "\twide_area: %i\n"
			       "\tmulticast: %i\n"
			       "\tcached: %i",
			  n, type, domain, host_name, p, a, t,
			  avahi_string_list_get_service_cookie(txt),
			  !!(flags & AVAHI_LOOKUP_RESULT_LOCAL),
			  !!(flags & AVAHI_LOOKUP_RESULT_OUR_OWN),
			  !!(flags & AVAHI_LOOKUP_RESULT_WIDE_AREA),
			  !!(flags & AVAHI_LOOKUP_RESULT_MULTICAST),
			  !!(flags & AVAHI_LOOKUP_RESULT_CACHED));

		if (!strcmp(name, n) && strlen(a)<20) { //TODO: Remove IPv6 avoider
			strcpy(address, a);
			port = p;
			avahi_simple_poll_quit(simple_poll);
		}

		avahi_free(t);
	}
	}

	avahi_service_resolver_free(r);
}

static void browse_callback(AvahiServiceBrowser *b, AvahiIfIndex interface,
			    AvahiProtocol protocol, AvahiBrowserEvent event,
			    const char *name, const char *type,
			    const char *domain,
			    AVAHI_GCC_UNUSED AvahiLookupResultFlags flags,
			    void *userdata) {

	assert(b);

	/* Called whenever a new services becomes available on the LAN or is
	 * removed from the LAN */

	switch (event) {
	case AVAHI_BROWSER_FAILURE:

		EXPIO_LOG_ERR("%s", avahi_strerror(avahi_client_errno(
					avahi_service_browser_get_client(b))));
		avahi_simple_poll_quit(simple_poll);
		return;

	case AVAHI_BROWSER_NEW:
		EXPIO_LOG(100, "NEW: service '%s' of type '%s' in domain '%s'",
			  name, type, domain);

		/* We ignore the returned resolver object. In the callback
		   function we free it. If the server is terminated before
		   the callback function is called the server will free
		   the resolver for us. */

		if (!(avahi_service_resolver_new(
			client, interface, protocol, name, type, domain,
			AVAHI_PROTO_UNSPEC, AVAHI_LOOKUP_NO_TXT,
			resolve_callback, NULL)))
			EXPIO_LOG_ERR(
			    "Failed to resolve service '%s': %s", name,
			    avahi_strerror(avahi_client_errno(client)));

		break;

	case AVAHI_BROWSER_REMOVE:
		EXPIO_LOG(100,
			  "REMOVE: service '%s' of type '%s' in domain '%s'",
			  name, type, domain);
		break;

	case AVAHI_BROWSER_ALL_FOR_NOW:
	case AVAHI_BROWSER_CACHE_EXHAUSTED:
		EXPIO_LOG_INFO("%s", event == AVAHI_BROWSER_CACHE_EXHAUSTED
					 ? "CACHE_EXHAUSTED"
					 : "ALL_FOR_NOW");
		break;
	}
}

static void client_callback(AvahiClient *c, AvahiClientState state,
			    AVAHI_GCC_UNUSED void *userdata) {
	assert(c);

	/* Called whenever the client or server state changes */

	if (state == AVAHI_CLIENT_FAILURE) {
		EXPIO_LOG_ERR("Server connection failure: %s",
			      avahi_strerror(avahi_client_errno(c)));
		avahi_simple_poll_quit(simple_poll);
	}
}

bool yapsc::tcp_zeroconf_discover(const char *n, char *a, int *p) {
	int error;
	bool ret = false;

	name = avahi_strdup(n);
	address = a;
	*address = '\0';

	/* Allocate main loop object */
	if (!(simple_poll = avahi_simple_poll_new())) {
		EXPIO_LOG_ERR("Failed to create simple poll object.");
		goto fail;
	}

	/* Allocate a new client */
	client = avahi_client_new(avahi_simple_poll_get(simple_poll),
				  AVAHI_CLIENT_IGNORE_USER_CONFIG,
				  client_callback, NULL, &error);

	/* Check wether creating the client object succeeded */
	if (!client) {
		EXPIO_LOG_ERR("Failed to create client: %s",
			      avahi_strerror(error));
		goto fail;
	}

	/* Create the service browser */
	if (!(sb = avahi_service_browser_new(
		  client, AVAHI_IF_UNSPEC, AVAHI_PROTO_UNSPEC, "_yapsc._tcp",
		  NULL, (AvahiLookupFlags)0, browse_callback, NULL))) {
		EXPIO_LOG_ERR("Failed to create service browser: %s",
			      avahi_strerror(avahi_client_errno(client)));
		goto fail;
	}

	/* Run the main loop */
	avahi_simple_poll_loop(simple_poll);

	if (*address) {
		ret = true;
		*p = port;
	}

fail:

	if (sb)
		avahi_service_browser_free(sb);

	if (client)
		avahi_client_free(client);

	if (simple_poll)
		avahi_simple_poll_free(simple_poll);

	return ret;
}

#ifdef DEBUG_MAIN
#include <stdio.h>

int main(AVAHI_GCC_UNUSED int argc, AVAHI_GCC_UNUSED char *argv[]) {
	char address[30];
	int port = 0;

	yapsc_lp_tcp_zeroconf_discover("ExampleName", address, &port);

	printf("%s:%d\n", address, port);

	return 0;
}
#endif
