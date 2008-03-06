#include <Python.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <assert.h>

#define CMSG_BUFSIZE 1000
/* int sendmsg(int s, struct msghdr *msg, int flags); */
static PyObject *my_sendmsg(PyObject *self, PyObject *args, PyObject *keywds) {
  int fd;
  int flags=0;
  int ret;
  struct msghdr msg = {0};
  struct sockaddr_in sa;
  struct iovec iov[1];
  char cmsgbuf[CMSG_BUFSIZE];
  PyObject *ancillary = NULL;
  char *host = NULL;
  int port = 0;

  static char *kwlist[] = {"fd",
			   "data",
			   "host", "port",
			   "flags",
			   "ancillary",
			   NULL};

  if (!PyArg_ParseTupleAndKeywords(args, keywds, "it#|siiO", kwlist,
				   &fd,
				   &iov[0].iov_base,
				   &iov[0].iov_len,
				   &host,
				   &port,
				   &flags,
				   &ancillary))
    return NULL;

  if (host || port) {
    memset(&sa, 0, sizeof(sa));
    sa.sin_family = AF_INET;
    sa.sin_port = htons(port);

    if (host) {
      int address_valid;

      address_valid = inet_aton(host, &sa.sin_addr);
      if (!address_valid)
	return NULL;
    }

    msg.msg_name = &sa;
    msg.msg_namelen = sizeof(sa);
  }

  msg.msg_iov = iov;
  msg.msg_iovlen = 1;

  msg.msg_control = cmsgbuf;
  msg.msg_controllen = 0;

  if (ancillary) {
    struct cmsghdr *cur;
    size_t real_controllen = 0;
    PyObject *iterator;
    PyObject *item;

    msg.msg_controllen = sizeof(cmsgbuf);
    cur = CMSG_FIRSTHDR(&msg);

    iterator = PyObject_GetIter(ancillary);
    if (!iterator)
      return NULL; // TODO proper error?

    while ((item = PyIter_Next(iterator))) {
      PyObject *rest;

      if (!cur)
	return NULL; // TODO proper error, leaks

      if (!PyArg_ParseTuple(item, "iiO",
			    &cur->cmsg_level,
			    &cur->cmsg_type,
			    &rest))
	return NULL; // TODO leaks
      if (cur->cmsg_level == SOL_IP
	  && cur->cmsg_type == IP_PKTINFO) {
	struct in_pktinfo *info = (void*)CMSG_DATA(cur);
	char *spec_dst;
	char *addr;

	if (!PyArg_ParseTuple(rest, "iss",
			      &info->ipi_ifindex,
			      &spec_dst,
			      &addr))
	  return NULL; // TODO leaks

	if (!inet_aton(spec_dst, &info->ipi_spec_dst))
	  return NULL; // TODO leaks
	if (!inet_aton(addr, &info->ipi_addr))
	  return NULL; // TODO leaks

	cur->cmsg_len = CMSG_LEN(sizeof *info);
	real_controllen += CMSG_SPACE(sizeof *info);
      } else {
	char *data;
	ssize_t len;

	if (PyString_AsStringAndSize(rest, &data, &len))
	  return NULL; // TODO leaks

	memcpy(CMSG_DATA(cur), data, len);
	cur->cmsg_len = CMSG_LEN(len);
	real_controllen += CMSG_SPACE(len);
     }

      cur = CMSG_NXTHDR(&msg, cur);
      Py_DECREF(item);
    }
    Py_DECREF(iterator);

    if (PyErr_Occurred())
      return NULL;

    msg.msg_controllen = real_controllen;
  }

  ret = sendmsg(fd, &msg, flags);
  if (ret<0) {
    PyErr_SetFromErrno(PyExc_OSError);
    return NULL;
  }

  Py_INCREF(Py_None);
  return Py_None;
}

static PyMethodDef my_methods[] = {
  {
    "sendmsg", (PyCFunction)my_sendmsg, METH_VARARGS|METH_KEYWORDS,
    "sendmsg(fd, data, flags=0, ancillary=[])"
  },

  {NULL, NULL, 0, NULL}
};

void initsendmsg(void) {
  Py_InitModule("sendmsg", my_methods);
}
