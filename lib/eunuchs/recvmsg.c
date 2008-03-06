#include <Python.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <assert.h>

#define CMSG_BUFSIZE 1000
/* int recvmsg(int s, struct msghdr *msg, int flags); */
static PyObject *my_recvmsg(PyObject *self, PyObject *args, PyObject *keywds) {
  int fd;
  int flags=0;
  size_t maxsize=8192;
  int ret;
  struct msghdr msg;
  struct sockaddr_in sa = {0};
  struct iovec iov[1];
  char cmsgbuf[CMSG_BUFSIZE];
  PyObject *ancillary;

  static char *kwlist[] = {"fd", "flags", "maxsize", "ancillary", NULL};

  if (!PyArg_ParseTupleAndKeywords(args, keywds, "i|iiO", kwlist,
				   &fd,
				   &flags,
				   &maxsize,
				   &ancillary
				   ))
    return NULL;

  msg.msg_name = &sa;
  msg.msg_namelen = sizeof(sa);

  iov[0].iov_len = maxsize;
  iov[0].iov_base = malloc(maxsize);
  if (!iov[0].iov_base) {
    PyErr_SetFromErrno(PyExc_OSError);
    return NULL;
  }
  msg.msg_iov = iov;
  msg.msg_iovlen = 1;

  memset(cmsgbuf, 0, sizeof(cmsgbuf));
  msg.msg_control = cmsgbuf;
  msg.msg_controllen = sizeof(cmsgbuf);

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

      {
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

  ret = recvmsg(fd, &msg, flags);
  if (ret<0) {
    PyErr_SetFromErrno(PyExc_OSError);
    free(iov[0].iov_base);
    return NULL;
  }

  ancillary = PyList_New(0);
  if (!ancillary) {
    free(iov[0].iov_base);
    return NULL;
  }

  {
    struct cmsghdr *cur;

    for (cur=CMSG_FIRSTHDR(&msg); cur; cur=CMSG_NXTHDR(&msg, cur)) {
      PyObject *entry;

      assert(cur->cmsg_len >= sizeof(struct cmsghdr));

      if (cur->cmsg_level == SOL_IP
	  && cur->cmsg_type == IP_PKTINFO) {
	struct in_pktinfo *info = (void*)CMSG_DATA(cur);
	PyObject *spec_dst;
	PyObject *addr;

	assert( cur->cmsg_len == sizeof(struct cmsghdr) + sizeof(struct in_pktinfo));

	spec_dst = Py_BuildValue("s", inet_ntoa(info->ipi_spec_dst));
	addr = Py_BuildValue("s", inet_ntoa(info->ipi_addr));
	entry = Py_BuildValue("(ii(iNN))",
			      cur->cmsg_level,
			      cur->cmsg_type,
			      info->ipi_ifindex,
			      spec_dst,
			      addr);
      } else {
	entry = Py_BuildValue("(iis#)",
			      cur->cmsg_level,
			      cur->cmsg_type,
			      CMSG_DATA(cur),
			      cur->cmsg_len - sizeof(struct cmsghdr)
			      );
      }
      if (PyList_Append(ancillary, entry) < 0) {
	Py_DECREF(ancillary);
	Py_DECREF(entry);
	free(iov[0].iov_base);
	return NULL;
      }
    }
  }

  {
    PyObject *r;
    r = Py_BuildValue("s#(si)iO",
		      iov[0].iov_base, ret,
		      inet_ntoa(sa.sin_addr), ntohs(sa.sin_port),
		      msg.msg_flags,
		      ancillary
		      );
    free(iov[0].iov_base);
    return r;
  }
}

static PyMethodDef my_methods[] = {
  {
    "recvmsg", (PyCFunction)my_recvmsg, METH_VARARGS|METH_KEYWORDS,
    "recvmsg(fd, flags=0) -> data, (host, port), flags, (...)    TODO"
  },

  {NULL, NULL, 0, NULL}
};

void initrecvmsg(void) {
  Py_InitModule("recvmsg", my_methods);
}
