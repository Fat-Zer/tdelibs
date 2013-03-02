#define DBUS_API_SUBJECT_TO_CHANGE
#include <dbus/dbus.h>
#include <stdbool.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

void reply_Bool(DBusMessage* msg, DBusConnection* conn, int value) {
	DBusMessage* reply;
	DBusMessageIter args;
	const char* member = dbus_message_get_member(msg);
	dbus_uint32_t serial = 0;

	// create a reply from the message
	reply = dbus_message_new_method_return(msg);

	// add the arguments to the reply
	dbus_message_iter_init_append(reply, &args);
	if (!dbus_message_iter_append_basic(&args, DBUS_TYPE_BOOLEAN, &value)) {
		fprintf(stderr, "[tde_dbus_hardwarecontrol] %s: dbus_message_iter_append_basic failed\n", member);
		return;
	}

	// send the reply && flush the connection
	if (!dbus_connection_send(conn, reply, &serial)) {
		fprintf(stderr, "[tde_dbus_hardwarecontrol] %s: dbus_connection_send failed\n", member);
		return;
	}
	dbus_connection_flush(conn);

	// free the reply
	dbus_message_unref(reply);
}

void reply_CanSetGivenPath(DBusMessage* msg, DBusConnection* conn, const char* param) {
	DBusMessage* reply;
	DBusMessageIter args;
	const char* member = dbus_message_get_member(msg);
	dbus_uint32_t serial = 0;
	int writable = false;

	// check if path is writable
	int rval = access (param, W_OK);
	if (rval == 0) {
		writable = true;
	}

	// create a reply from the message
	reply = dbus_message_new_method_return(msg);

	// add the arguments to the reply
	dbus_message_iter_init_append(reply, &args);
	if (!dbus_message_iter_append_basic(&args, DBUS_TYPE_BOOLEAN, &writable)) {
		fprintf(stderr, "[tde_dbus_hardwarecontrol] %s: dbus_message_iter_append_basic failed\n", member);
		return;
	}

	// send the reply && flush the connection
	if (!dbus_connection_send(conn, reply, &serial)) {
		fprintf(stderr, "[tde_dbus_hardwarecontrol] %s: dbus_connection_send failed\n", member);
		return;
	}
	dbus_connection_flush(conn);

	// free the reply
	dbus_message_unref(reply);
}

void reply_SetGivenPath(DBusMessage* msg, DBusConnection* conn, const char* param, const char* contents) {
	DBusMessage* reply;
	DBusMessageIter args;
	const char* member = dbus_message_get_member(msg);
	dbus_uint32_t serial = 0;
	int writable = false;
	int written = false;

	// check if path is writable
	int rval = access (param, W_OK);
	if (rval == 0) {
		writable = true;
	}

	if (writable) {
		FILE *node = fopen(param, "w");
		if (node != NULL) {
			if (fputs(contents, node) != EOF) {
				written = true;
			}
			if (fclose(node) == EOF) {
				// Error!
			}
		}
	}

	// create a reply from the message
	reply = dbus_message_new_method_return(msg);

	// add the arguments to the reply
	dbus_message_iter_init_append(reply, &args);
	if (!dbus_message_iter_append_basic(&args, DBUS_TYPE_BOOLEAN, &written)) {
		fprintf(stderr, "[tde_dbus_hardwarecontrol] %s: dbus_message_iter_append_basic failed\n", member);
		return;
	}

	// send the reply && flush the connection
	if (!dbus_connection_send(conn, reply, &serial)) {
		fprintf(stderr, "[tde_dbus_hardwarecontrol] %s: dbus_connection_send failed\n", member);
		return;
	}
	dbus_connection_flush(conn);

	// free the reply
	dbus_message_unref(reply);
}

void reply_CanSetCPUGovernor(DBusMessage* msg, DBusConnection* conn) {
	DBusMessageIter args;
	const char* member = dbus_message_get_member(msg);
	dbus_int32_t cpunum;
	char path[256];

	// read the arguments
	if (!dbus_message_iter_init(msg, &args)) {
		fprintf(stderr, "[tde_dbus_hardwarecontrol] %s: no argument supplied\n", member);
	}
	else if (DBUS_TYPE_INT32 != dbus_message_iter_get_arg_type(&args)) {
		fprintf(stderr, "[tde_dbus_hardwarecontrol] %s: argument not 32-bit integer\n", member);
	}
	else {
		dbus_message_iter_get_basic(&args, &cpunum);
	}

	snprintf(path, 256, "/sys/devices/system/cpu/cpu%d/cpufreq/scaling_governor", cpunum);
	reply_CanSetGivenPath(msg, conn, path);
}

void reply_SetCPUGovernor(DBusMessage* msg, DBusConnection* conn) {
	DBusMessageIter args;
	const char* member = dbus_message_get_member(msg);
	dbus_int32_t cpunum = -1;
	char* governor = NULL;
	char path[256];

	// read the arguments
	if (!dbus_message_iter_init(msg, &args)) {
		fprintf(stderr, "[tde_dbus_hardwarecontrol] %s: no arguments supplied\n", member);
	}
	else if (DBUS_TYPE_INT32 != dbus_message_iter_get_arg_type(&args)) {
		fprintf(stderr, "[tde_dbus_hardwarecontrol] %s: first argument not 32-bit integer\n", member);
	}
	else {
		dbus_message_iter_get_basic(&args, &cpunum);
	}

	if (!dbus_message_iter_next(&args)) {
		fprintf(stderr, "[tde_dbus_hardwarecontrol] %s: second argument not supplied\n", member);
	}
	else if (DBUS_TYPE_STRING != dbus_message_iter_get_arg_type(&args)) {
		fprintf(stderr, "[tde_dbus_hardwarecontrol] %s: second argument not string\n", member);
	}
	else {
		dbus_message_iter_get_basic(&args, &governor);
	}

	snprintf(path, 256, "/sys/devices/system/cpu/cpu%d/cpufreq/scaling_governor", cpunum);
	if ((cpunum>-1) && governor) {
		reply_SetGivenPath(msg, conn, path, governor);
	}
	else {
		reply_Bool(msg, conn, false);
	}
}

void reply_CanSetBrightness(DBusMessage* msg, DBusConnection* conn) {
	DBusMessageIter args;
	const char* member = dbus_message_get_member(msg);
	char* rawpath;
	char* safepath;
	char path[256];

	// read the arguments
	if (!dbus_message_iter_init(msg, &args)) {
		fprintf(stderr, "[tde_dbus_hardwarecontrol] %s: no argument supplied\n", member);
	}
	else if (DBUS_TYPE_STRING != dbus_message_iter_get_arg_type(&args)) {
		fprintf(stderr, "[tde_dbus_hardwarecontrol] %s: argument not string\n", member);
	}
	else {
		dbus_message_iter_get_basic(&args, &rawpath);
	}

	safepath = realpath(rawpath, NULL);

	if (safepath &&
		(strstr(safepath, "/sys/devices") == safepath) &&
		(strstr(safepath, "/brightness") == (safepath+strlen(safepath)-strlen("/brightness")))
		) {
			reply_CanSetGivenPath(msg, conn, safepath);
	}
	else {
		reply_Bool(msg, conn, false);
	}

	free(safepath);
}

void reply_SetBrightness(DBusMessage* msg, DBusConnection* conn) {
	DBusMessageIter args;
	const char* member = dbus_message_get_member(msg);
	char* rawpath;
	char* safepath;
	char* brightness;
	char path[256];

	// read the arguments
	if (!dbus_message_iter_init(msg, &args)) {
		fprintf(stderr, "[tde_dbus_hardwarecontrol] %s: no arguments supplied\n", member);
	}
	else if (DBUS_TYPE_STRING != dbus_message_iter_get_arg_type(&args)) {
		fprintf(stderr, "[tde_dbus_hardwarecontrol] %s: first argument not string\n", member);
	}
	else {
		dbus_message_iter_get_basic(&args, &rawpath);
	}

	if (!dbus_message_iter_next(&args)) {
		fprintf(stderr, "[tde_dbus_hardwarecontrol] %s: second argument not supplied\n", member);
	}
	else if (DBUS_TYPE_STRING != dbus_message_iter_get_arg_type(&args)) {
		fprintf(stderr, "[tde_dbus_hardwarecontrol] %s: second argument not string\n", member);
	}
	else {
		dbus_message_iter_get_basic(&args, &brightness);
	}

	safepath = realpath(rawpath, NULL);

	if (safepath && brightness &&
		(strstr(safepath, "/sys/devices") == safepath) &&
		(strstr(safepath, "/brightness") == (safepath+strlen(safepath)-strlen("/brightness")))
		) {
			reply_SetGivenPath(msg, conn, safepath, brightness);
	}
	else {
		reply_Bool(msg, conn, false);
	}

	free(safepath);
}

void listen() {
	DBusMessage* msg;
	DBusMessage* reply;
	DBusMessageIter args;
	DBusConnection* conn;
	DBusError err;
	int ret;
	char* param;

	fprintf(stderr, "[tde_dbus_hardwarecontrol] Listening...\n");

	// initialise the error structure
	dbus_error_init(&err);

	// connect to the bus and check for errors
	conn = dbus_bus_get(DBUS_BUS_SYSTEM, &err);
	if (dbus_error_is_set(&err)) {
		fprintf(stderr, "[tde_dbus_hardwarecontrol] Connection failed with error '%s'\n", err.message);
		dbus_error_free(&err);
	}
	if (NULL == conn) {
		fprintf(stderr, "[tde_dbus_hardwarecontrol] No connection, exiting!\n");
		exit(1);
	}

	// request our name on the bus and check for errors
	ret = dbus_bus_request_name(conn, "org.trinitydesktop.hardwarecontrol", DBUS_NAME_FLAG_REPLACE_EXISTING , &err);
	if (dbus_error_is_set(&err)) {
		fprintf(stderr, "[tde_dbus_hardwarecontrol] Name request failed with error '%s'\n", err.message);
		dbus_error_free(&err);
	}
	if (DBUS_REQUEST_NAME_REPLY_PRIMARY_OWNER != ret) {
		fprintf(stderr, "[tde_dbus_hardwarecontrol] Not primary owner (%d), exiting!\n", ret);
		exit(1);
	}

	// loop, testing for new messages
	while (true) {
		// non blocking read of the next available message
		dbus_connection_read_write(conn, 1000); // block for up to 1 second
		msg = dbus_connection_pop_message(conn);
	
		// loop again if we haven't got a message
		if (NULL == msg) {
			continue;
		}

		// check this is a method call for the right interface & method
		if (dbus_message_is_method_call(msg, "org.trinitydesktop.hardwarecontrol.CPUGovernor", "CanSetCPUGovernor")) {
			reply_CanSetCPUGovernor(msg, conn);
		}
		else if (dbus_message_is_method_call(msg, "org.trinitydesktop.hardwarecontrol.CPUGovernor", "SetCPUGovernor")) {
			reply_SetCPUGovernor(msg, conn);
		}
		else if (dbus_message_is_method_call(msg, "org.trinitydesktop.hardwarecontrol.Brightness", "CanSetBrightness")) {
			reply_CanSetBrightness(msg, conn);
		}
		else if (dbus_message_is_method_call(msg, "org.trinitydesktop.hardwarecontrol.Brightness", "SetBrightness")) {
			reply_SetBrightness(msg, conn);
		}
		else {
			fprintf(stderr, "[tde_dbus_hardwarecontrol] Unknown method '%s' called on interface '%s', ignoring\n", dbus_message_get_member(msg), dbus_message_get_interface(msg));
		}
		
		// free the message
		dbus_message_unref(msg);
	}

	// close the connection
	dbus_connection_close(conn);
}

int main(int argc, char** argv) {
	listen();
	return 0;
}
