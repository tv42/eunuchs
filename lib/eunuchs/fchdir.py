import warnings
warnings.warn("eunuchs.fchdir is deprecated, please use os.fchdir() directly.",
              stacklevel=2)

from os import fchdir
