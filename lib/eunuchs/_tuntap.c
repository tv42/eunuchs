#include <Python.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <linux/if.h>
#include <linux/if_tun.h>

static PyObject *my_opentuntap(PyObject *self, PyObject *args, PyObject *keywds) {
  char *name = NULL;
  int ethernet = 0;
  int packetinfo = 1;
  int err;
  int fd;
  struct ifreq ifr;

  static char *kwlist[] = {"name",
			   "ethernet",
			   "packetinfo",
			   NULL};

  if (!PyArg_ParseTupleAndKeywords(args, keywds, "|zii", kwlist,
				   &name,
				   &ethernet,
				   &packetinfo))
    return NULL;


  fd = open("/dev/net/tun", O_RDWR);
  if (fd<0) {
    PyErr_SetFromErrno(PyExc_OSError);
    return NULL;
  }

  memset(&ifr, 0, sizeof(ifr));

  if (ethernet) {
    ifr.ifr_flags = IFF_TAP;
  } else {
    ifr.ifr_flags = IFF_TUN;
  }

  if (!packetinfo)
    ifr.ifr_flags |= IFF_NO_PI;

  if(name)
    strncpy(ifr.ifr_name, name, IFNAMSIZ);

  err = ioctl(fd, TUNSETIFF, (void *) &ifr);
  if (err < 0) {
    PyErr_SetFromErrno(PyExc_OSError);
    close(fd);
    return NULL;
  }

  return Py_BuildValue("is", fd, ifr.ifr_name);
}

static PyMethodDef my_methods[] = {
  {
    "opentuntap", (PyCFunction)my_opentuntap, METH_VARARGS|METH_KEYWORDS,
    "opentuntap([name[, ethernet[, packetinfo]]]) -> (fd, fd)"
  },

  {NULL, NULL, 0, NULL}
};

/* Utility function to add integer constant to module dictionary */
static void insertConstant(PyObject *d, char *name, unsigned long code)
{
  PyObject *u = PyString_FromString(name);
  PyObject *v = PyInt_FromLong((long) code);

  if (u && v) {
    /* insert in modules dict */
    PyDict_SetItem(d, u, v);
  }
  Py_XDECREF(u);
  Py_XDECREF(v);
}

void init_tuntap(void) {
  PyObject *module, *dict;

  module = Py_InitModule("_tuntap", my_methods);
  dict = PyModule_GetDict(module);
  if (!dict) {
    return;
  }

  insertConstant(dict, "TUN_PKT_STRIP", TUN_PKT_STRIP);
}
