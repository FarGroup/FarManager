
from os.path import join, dirname, abspath

#: path to Encyclopedia sources root
ROOT_DIR        = abspath(join(dirname(__file__), "../../.."))

DEST            = join(ROOT_DIR, "build")
DEST_CHM        = join(DEST, "chm")
BUILD_LOG       = join(DEST, "tool.make_chm.log")


#DEST_INET       = join(DEST, "inet")
#DEST_INET_RU    = join(DEST_INET, "ru")
#DEST_INET_EN    = join(DEST_INET, "en")
#DEST_CHM_RU     = join(DEST_CHM, "ru")
#DEST_CHM_EN     = join(DEST_CHM, "en")

#META_RU            = join(DEST_CHM_RU, "meta")
#META_EN            = join(DEST_CHM_EN, "meta")

