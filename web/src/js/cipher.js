
import nacl from 'tweetnacl';


function serializeKeys(myKeys) {
  return JSON.stringify({
    secretKey: Buffer.from(myKeys.secretKey).toString('base64'),
    publicKey: Buffer.from(myKeys.publicKey).toString('base64')
  })
}

function deserializeKeys(data) {
  var myKeys = JSON.parse(data);
  return {
    secretKey: new Uint8Array(Buffer.from(myKeys.secretKey, 'base64')),
    publicKey: new Uint8Array(Buffer.from(myKeys.publicKey, 'base64'))
  }
}

function init() {

  try {
    app.keys = deserializeKeys(localStorage.getItem("cryptoKeys"));
    if(!app.keys.secretKey || app.keys.secretKey.length !== 64 || !app.keys.publicKey || app.keys.publicKey.length != 32) throw new Error("No keys or invalid cryptographic keys found in localStorage. Will generate a new pair.");

    console.log("Cryptographic keys successfully loaded from localStorage");
  } catch(e) {
    console.warn(e);
    app.keys = nacl.sign.keyPair();
    localStorage.setItem("cryptoKeys", serializeKeys(app.keys));
  }

}

function stringToUint8Array(str) {
  return new Uint8Array(Buffer.from(str, 'utf8'));
}

function sign(msg) {
  if(typeof msg === 'string') {
    msg = stringToUint8Array(msg);
  }
  return nacl.sign.detached(msg, app.keys.secretKey);
}

function verify(msg, signature, pubKey) {
  if(typeof msg === 'string') {
    msg = stringToUint8Array(msg);
  }
  return nacl.sign.detached.verify(msg, signature, pubKey);
}

function getSecretKey() {
  return app.keys.secretKey;
}

function getPublicKey() {
  return app.keys.publicKey;
}

export default {
  init,
  getSecretKey,
  getPublicKey,
  sign,
  verify
}
