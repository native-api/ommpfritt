import datetime
import sys


def header():
    return f"""\
/**
 * Created by {sys.argv[0]}
 * Date: {datetime.datetime.now().date().isoformat()}
 * Time: {datetime.datetime.now().time().isoformat()}
 *
 * This file is autogenerated, don't change it manually,
 * any manual changes will get lost after next regeneration.
 */

"""