const fs = require('fs')
const os = require('os')
const path = require('path')

function cmake(a) {
  let extra = [];

  let userPackageLocal = path.join(os.homedir(), ".config/CPM/package-local.cmake");
  if(fs.existsSync(userPackageLocal)) {
    extra.push(`-C ${userPackageLocal}`);
  } else {
    console.log("Warning: no local CPM configuration");
  }

  if(a.opts) {
    if(Array.isArray(a.opts))
      a.opts = a.opts.join(' ');
  } else {
    a.opts = "";
  }


  return { options: { args: `${extra.join(' ')} -DTOOLCHAIN=${a.tc} -DBUILD_CONFIG=${a.c} ${a.opts}` } };
}

module.exports = { cmake }
