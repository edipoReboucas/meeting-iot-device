// Load the TCP Library
net = require('net');

// Keep track of the chat clients
var clients = [];

const COMMAND_REGISTER = parseInt('1', 2);
const COMMAND_RESERVE  = parseInt('10', 2);
const COMMAND_STATUS   = parseInt('11', 2);

const STATUS_ROOM_UNDEFINED = parseInt('00', 2);
const STATUS_ROOM_FREE = parseInt('01', 2);
const STATUS_ROOM_IN_MEETING = parseInt('10', 2);
const STATUS_ROOM_MEETING_SOON = parseInt('11', 2);


// Start a TCP Server
net.createServer(function (socket) {
  //Set a name for the client

  //Push on the client array
  clients.push(socket);

  //When receive data
  socket.on('data', function (buffer) {

    console.log('buffer', buffer);
    console.log('length', buffer.length);
    console.log('command', buffer[0]);
    console.log('param',   buffer[1]);

    const command = buffer[0];
    const param = buffer[1];
    
    switch(command) {
      case COMMAND_REGISTER:
        socket.name = param;
	setTimeout(function() { //consultando status da sala
	  socket.write(Buffer.from([COMMAND_STATUS, STATUS_ROOM_FREE], 2));
	}, 2000);
        break;
      case COMMAND_RESERVE:
	setTimeout(function() { //reservando e retornando status da sala
	  socket.write(Buffer.from([COMMAND_STATUS, 0x2], 2));
	}, 10000);
        break;
    }
  });

  socket.on('error', function () {
    clients.splice(clients.indexOf(socket), 1);
  });
 
  //When connection ends
  socket.on('end', function () {
    clients.splice(clients.indexOf(socket), 1);
  });
  
}).listen(8080);

// Put a friendly message on the terminal of the server.
console.log('Agendador server running at port 8080\n');
