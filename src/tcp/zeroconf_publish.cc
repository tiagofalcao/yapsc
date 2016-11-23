#include "yapsc_tcp.h"
#include <assert.h>
#include <expio.h>
#include <stdbool.h>

#include <avahi-client/client.h>
#include <avahi-client/publish.h>

#include <avahi-common/error.h>
#include <avahi-common/malloc.h>
#include <avahi-common/thread-watch.h>

static AvahiEntryGroup *group = NULL;
static AvahiClient *client = NULL;
static AvahiThreadedPoll *threaded_poll = NULL;

static char *name = NULL;
static int port = 0;

static void create_services(AvahiClient *c);

static void entry_group_callback(AvahiEntryGroup *g, AvahiEntryGroupState state,
				 AVAHI_GCC_UNUSED void *userdata) {
	assert(g == group || group == NULL);
	group = g;

	/* Called whenever the entry group state changes */

	switch (state) {
	case AVAHI_ENTRY_GROUP_ESTABLISHED:
		/* The entry group has been established successfully */
		EXPIO_LOG_INFO("Service '%s' successfully established.", name);
		break;

	case AVAHI_ENTRY_GROUP_COLLISION: {
		EXPIO_LOG_ERR("Service name collision");

		avahi_threaded_poll_quit(threaded_poll);
		break;
	}

	case AVAHI_ENTRY_GROUP_FAILURE:
		EXPIO_LOG_ERR("Entry group failure: %s",
				avahi_strerror(avahi_client_errno(
				    avahi_entry_group_get_client(g))));

		/* Some kind of failure happened while we were registering our
		 * services */
		avahi_threaded_poll_quit(threaded_poll);
		break;

	case AVAHI_ENTRY_GROUP_UNCOMMITED:
	case AVAHI_ENTRY_GROUP_REGISTERING:;
	}
}

static void create_services(AvahiClient *c) {
	char *n, r[128];
	int ret;
	assert(c);

	/* If this is the first time we're called, let's create a new
	 * entry group if necessary */

	if (!group)
		if (!(group = avahi_entry_group_new(c, entry_group_callback,
						    NULL))) {
			EXPIO_LOG_ERR("avahi_entry_group_new() failed: %s",
					avahi_strerror(avahi_client_errno(c)));
			goto fail;
		}

	/* If the group is empty (either because it was just created, or
	 * because it was reset previously, add our entries.  */

	if (avahi_entry_group_is_empty(group)) {
		EXPIO_LOG_INFO("Adding service '%s'", name);

		/* Create some random TXT data */
		snprintf(r, sizeof(r), "random=%i", rand());

		if ((ret = avahi_entry_group_add_service(
			 group, AVAHI_IF_UNSPEC, AVAHI_PROTO_UNSPEC,
			 AVAHI_PUBLISH_USE_MULTICAST, name, "_yapsc._tcp", NULL,
			 NULL, port, NULL, NULL, NULL)) < 0) {

			if (ret == AVAHI_ERR_COLLISION)
				goto fail;

			fprintf(stderr, "Failed to add _yapsc._tcp service: %s",
				avahi_strerror(ret));
			goto fail;
		}

		/* Tell the server to register the service */
		if ((ret = avahi_entry_group_commit(group)) < 0) {
			EXPIO_LOG_ERR("Failed to commit entry group: %s",
					avahi_strerror(ret));
			goto fail;
		}
	}

	return;

fail:
	avahi_threaded_poll_quit(threaded_poll);
}

static void client_callback(AvahiClient *c, AvahiClientState state,
			    AVAHI_GCC_UNUSED void *userdata) {
	assert(c);

	/* Called whenever the client or server state changes */

	switch (state) {
	case AVAHI_CLIENT_S_RUNNING:

		/* The server has startup successfully and registered its host
		 * name on the network, so it's time to create our services */
		create_services(c);
		break;

	case AVAHI_CLIENT_FAILURE:

		EXPIO_LOG_ERR("Client failure: %s",
				avahi_strerror(avahi_client_errno(c)));
		avahi_threaded_poll_quit(threaded_poll);

		break;

	case AVAHI_CLIENT_S_COLLISION:

	/* Let's drop our registered services. When the server is back
	 * in AVAHI_SERVER_RUNNING state we will register them
	 * again with the new host name. */

	case AVAHI_CLIENT_S_REGISTERING:

		/* The server records are now being established. This
		 * might be caused by a host name change. We need to wait
		 * for our own records to register until the host name is
		 * properly esatblished. */

		if (group)
			avahi_entry_group_reset(group);

		break;

	case AVAHI_CLIENT_CONNECTING:;
	}
}

void yapsc::tcp_zeroconf_publish_shutdown(void) {
	if (threaded_poll) {
		avahi_threaded_poll_stop(threaded_poll);
	}
	if (client) {
		avahi_client_free(client);
		client = NULL;
	}
	if (threaded_poll) {
		avahi_threaded_poll_free(threaded_poll);
		threaded_poll = NULL;
	}
	if (name) {
		avahi_free(name);
		name = NULL;
	}
}

bool yapsc::tcp_zeroconf_publish_init(const char *n, int p) {
	int error;
	name = avahi_strdup(n);
	port = p;

	if (!(threaded_poll = avahi_threaded_poll_new())) {
		EXPIO_LOG_ERR("Failed to create avahi poll object.");
		goto fail;
	}

	if (!(client = avahi_client_new(avahi_threaded_poll_get(threaded_poll),
					AVAHI_CLIENT_IGNORE_USER_CONFIG,
					client_callback, NULL, &error))) {
		EXPIO_LOG_ERR("Failed to create client: %s",
				avahi_strerror(error));
		goto fail;
	}

	/* Finally, start the event loop thread */
	if (avahi_threaded_poll_start(threaded_poll) < 0) {
		EXPIO_LOG_ERR("Failed to run avahi pool");
		goto fail;
	}
	EXPIO_LOG_INFO("Published.");
	return true;
fail:
	yapsc::tcp_zeroconf_publish_shutdown();
	return false;
}
