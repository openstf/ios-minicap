const http = require('http')
const path = require('path')
const net = require('net')

const express = require('express')
const WebSocketServer = require('ws').Server
const debug = require('debug')('example')
const { BannerParser, FrameParser } = require('minicap')

const app = express()

const PORT = process.env.PORT || 9002

app.use(express.static(path.join(__dirname, '/public')))

const server = http.createServer(app)
const wss = new WebSocketServer({ server: server })

wss.on('connection', (ws) => {
  console.info('Got a client')

  const stream = net.connect({
    port: 12345
  })

  stream.on('error', (err) => {
    console.error(err)
    console.error('Be sure to run `ios-minicap --port 12345`')
    process.exit(1)
  })

  let bannerParsed = false
  const bannerParser = new BannerParser()
  const frameParser = new FrameParser()

  function onBannerAvailable (banner) {
    debug('banner', banner)
  }

  function onFrameAvailable (frame) {
    ws.send(frame.buffer, {
      binary: true
    })
  }

  function tryParse () {
    for (let chunk; (chunk = stream.read());) {
      do {
        if (!bannerParsed) {
          const result = bannerParser.parse(chunk)
          if (result.state === BannerParser.COMPLETE) {
            bannerParsed = true
            onBannerAvailable(result.take())
          }
          chunk = result.rest
        } else {
          const result = frameParser.parse(chunk)
          if (result.state === FrameParser.COMPLETE) {
            onFrameAvailable(result.take())
          }
          chunk = result.rest
        }
      } while (chunk.length)
    }
  }

  stream.on('readable', tryParse)
  tryParse()

  ws.on('close', () => {
    console.info('Lost a client')
    stream.end()
  })
})

server.listen(PORT)
console.info(`Listening on port ${PORT}`)
