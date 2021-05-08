/*
 Mining Pool Agent

 Copyright (C) 2016  BTC.COM

 This program is free software: you can redistribute it and/or modify
 it under the terms of the GNU General Public License as published by
 the Free Software Foundation, either version 3 of the License, or
 (at your option) any later version.

 This program is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */
#include "gtest/gtest.h"
#include "Utils.h"
#include "bitcoin/ServerBitcoin.h"

TEST(Server, SessionIDManager) {
  SessionIDManager m;
  uint16_t id;

  // fill all session ids
  for (uint32_t i = 0; i <= AGENT_MAX_SESSION_ID; i++) {
    ASSERT_EQ(m.allocSessionId(&id), true);
    ASSERT_EQ(id, i);
  }

  // it's full now
  ASSERT_EQ(m.ifFull(), true);
  ASSERT_EQ(m.allocSessionId(&id), false);

  // free the fisrt one
  id = 0u;
  m.freeSessionId(id);
  ASSERT_EQ(m.ifFull(), false);

  ASSERT_EQ(m.allocSessionId(&id), true);
  ASSERT_EQ(id, 0);
  ASSERT_EQ(m.ifFull(), true);

  // free the last one
  id = AGENT_MAX_SESSION_ID;
  m.freeSessionId(id);
  ASSERT_EQ(m.ifFull(), false);

  ASSERT_EQ(m.allocSessionId(&id), true);
  ASSERT_EQ(id, AGENT_MAX_SESSION_ID);
  ASSERT_EQ(m.ifFull(), true);
}

TEST(Server, StratumMessageBitcoin_isValid) {
  {
    string line;
    StratumMessageBitcoin smsg(line);
    ASSERT_EQ(smsg.isValid(), false);
  }

  {
    string line = "[]";  // shoud be objest: {}
    StratumMessageBitcoin smsg(line);
    ASSERT_EQ(smsg.isValid(), false);
  }
}

TEST(Server, StratumMessageBitcoin_getExtraNonce1AndExtraNonce2Size) {
  string line = "{\"id\": 1, \"result\": [ [ [\"mining.set_difficulty\", \"b4b6693b72a50c7116db18d6497cac52\"], [\"mining.notify\", \"ae6812eb4cd7735a302a8a9dd95cf71f\"]], \"08000002\", 4], \"error\": null}";
  StratumMessageBitcoin smsg(line);

  ASSERT_EQ(smsg.isValid(), true);
  uint32_t nonce1 = 0u;
  int32_t n2Size = 0;
  ASSERT_EQ(smsg.getExtraNonce1AndExtraNonce2Size(&nonce1, &n2Size), true);
  ASSERT_EQ(nonce1, 0x08000002u);
  ASSERT_EQ(n2Size, 4);
}

TEST(Server, StratumMessageBitcoin_parseMiningAuthorize) {
  string line = "{\"params\": [\"btccom.kevin\", \"password\"], \"id\": 2, \"method\": \"mining.authorize\"}";
  StratumMessageBitcoin smsg(line);

  ASSERT_EQ(smsg.isValid(), true);
  string workerName;
  ASSERT_EQ(smsg.parseMiningAuthorize(workerName), true);
  ASSERT_EQ(workerName, "btccom.kevin");
}

TEST(Server, StratumMessageBitcoin_parseMiningSubscribe) {
  string line = "{\"id\": 1, \"method\": \"mining.subscribe\", \"params\": [\"bfgminer/4.4.0-32-gac4e9b3\", \"01ad557d\"]}";
  StratumMessageBitcoin smsg(line);

  ASSERT_EQ(smsg.isValid(), true);
  string minerAgent;
  ASSERT_EQ(smsg.parseMiningSubscribe(minerAgent), true);
  ASSERT_EQ(minerAgent, "bfgminer/4.4.0-32-gac4e9b3");
}

TEST(Server, StratumMessageBitcoin_parseMiningSetDifficulty) {
  string line = "{ \"id\": null, \"method\": \"mining.set_difficulty\", \"params\": [2]}";
  StratumMessageBitcoin smsg(line);

  ASSERT_EQ(smsg.isValid(), true);
  float diff = 0;
  ASSERT_EQ(smsg.parseMiningSetDifficulty(&diff), true);
  ASSERT_EQ(diff, 2);
}

TEST(Server, StratumMessageBitcoin_parseMiningSetVersionMask) {
  string line = "{\"id\":null,\"method\":\"mining.set_version_mask\",\"params\":[\"1fffe000\"]}";
  StratumMessageBitcoin smsg(line);

  ASSERT_EQ(smsg.isValid(), true);
  uint32_t versionMask = 0u;
  ASSERT_EQ(smsg.parseMiningSetVersionMask(&versionMask), true);
  ASSERT_EQ(versionMask, 0x1fffe000u);
}

TEST(Server, StratumMessageBitcoin_parseMiningConfigure) {
  string line = "{\"id\":3,\"method\":\"mining.configure\",\"params\":["
                    "[\"version-rolling\"],"
                    "{\"version-rolling.mask\":\"00ffee00\",\"version-rolling.min-bit-count\":2}"
                "]}";
  StratumMessageBitcoin smsg(line);

  ASSERT_EQ(smsg.isValid(), true);
  uint32_t versionMask = 0u;
  ASSERT_EQ(smsg.parseMiningConfigure(&versionMask), true);
  ASSERT_EQ(versionMask, 0x00ffee00u);
}

TEST(Server, StratumMessageBitcoin_parseAgentGetCapabilities) {
  string line = "{\"id\":\"" JSONRPC_GET_CAPS_REQ_ID "\",\"result\":{\"capabilities\":[\"aaa\", \"bbb\", \"verrol\", \"testing\"]}}";
  StratumMessageBitcoin smsg(line);
  ASSERT_EQ(smsg.isValid(), true);

  std::set<string> serverCaps;
  ASSERT_EQ(smsg.parseAgentGetCapabilities(serverCaps), true);

  ASSERT_EQ(serverCaps.size(), 4u);
  ASSERT_EQ(serverCaps.count("aaa"), 1u);
  ASSERT_EQ(serverCaps.count("bbb"), 1u);
  ASSERT_EQ(serverCaps.count("verrol"), 1u);
  ASSERT_EQ(serverCaps.count("testing"), 1u);
}

TEST(Server, StratumMessageBitcoin_parseMiningNotify) {
  {
    string line = "{\"params\": [\"1\", \"4d16b6f85af6e2198f44ae2a6de67f78487ae5611b77c6c0440b921e00000000\",\"01000000010000000000000000000000000000000000000000000000000000000000000000ffffffff20020862062f503253482f04b8864e5008\",\"072f736c7573682f000000000100f2052a010000001976a914d23fcdf86f7e756a64a7a9688ef9903327048ed988ac00000000\", [\"008d29799d7a2951689ab6de901a8a2878966fb3cf375db417b411fed76b54a2\", \"942d192aa135fc4efdc09166877468d7d199753f35095b10fbce656f7c14561d\"],\"00000002\", \"1c2ac4af\", \"504e86b9\", false], \"id\": null, \"method\": \"mining.notify\"}";
    StratumMessageBitcoin smsg(line);

    ASSERT_EQ(smsg.isValid(), true);
    StratumJobBitcoin sjob;
    ASSERT_EQ(smsg.parseMiningNotify(sjob), true);

    ASSERT_EQ(sjob.jobId_, 1u);
    ASSERT_EQ(sjob.prevHash_, "4d16b6f85af6e2198f44ae2a6de67f78487ae5611b77c6c0440b921e00000000");
    ASSERT_EQ(sjob.version_, 0x00000002);
    ASSERT_EQ(sjob.time_, 0x504e86b9u);
    ASSERT_EQ(sjob.isClean_, false);
  }

  {
    //
    // empty merkle tree array
    //
    string line = "{\"params\": [\"0\", \"4d16b6f85af6e2198f44ae2a6de67f78487ae5611b77c6c0440b921e00000000\",\"01000000010000000000000000000000000000000000000000000000000000000000000000ffffffff20020862062f503253482f04b8864e5008\",\"072f736c7573682f000000000100f2052a010000001976a914d23fcdf86f7e756a64a7a9688ef9903327048ed988ac00000000\", [],\"02000000\", \"1c2ac4af\", \"504e86b9\", true], \"id\": null, \"method\": \"mining.notify\"}";
    StratumMessageBitcoin smsg(line);

    ASSERT_EQ(smsg.isValid(), true);
    StratumJobBitcoin sjob;
    ASSERT_EQ(smsg.parseMiningNotify(sjob), true);

    ASSERT_EQ(sjob.jobId_, 0u);
    ASSERT_EQ(sjob.prevHash_, "4d16b6f85af6e2198f44ae2a6de67f78487ae5611b77c6c0440b921e00000000");
    ASSERT_EQ(sjob.version_, 0x02000000);
    ASSERT_EQ(sjob.time_, 0x504e86b9u);
    ASSERT_EQ(sjob.isClean_, true);
  }
}

TEST(Server, StratumMessageBitcoin_parseMiningSubmit) {
  // [Worker Name, Job ID, ExtraNonce2(hex), nTime(hex), nonce(hex)]
  string line = "{\"params\": [\"slush.miner1\", \"9\", \"00000001\", \"504e86ed\", \"b2957c02\"], \"id\": 4, \"method\": \"mining.submit\"}";
  StratumMessageBitcoin smsg(line);

  ASSERT_EQ(smsg.isValid(), true);
  ShareBitcoin share;
  ASSERT_EQ(smsg.parseMiningSubmit(share), true);
  ASSERT_EQ(share.jobId_, 9u);
  ASSERT_EQ(share.time_, 0x504e86edu);
  ASSERT_EQ(share.extraNonce2_, 0x00000001u);
  ASSERT_EQ(share.nonce_, 0xb2957c02u);
}

TEST(Server, StratumMessageBitcoin_getResultBoolean) {
  {
    string line = "{\"error\": null, \"id\": 4, \"result\": true}";
    StratumMessageBitcoin smsg(line);
    ASSERT_EQ(smsg.isValid(), true);
    ASSERT_EQ(smsg.getResultBoolean(), true);
  }

  {
    string line = "{\"error\": null, \"id\": 4, \"result\": false}";
    StratumMessageBitcoin smsg(line);
    ASSERT_EQ(smsg.isValid(), true);
    ASSERT_EQ(smsg.getResultBoolean(), false);
  }
}

TEST(Server, StratumMessageBitcoin_getId_isStringId) {
  {
    string line = "{\"error\": null, \"id\": 4, \"result\": true}";
    StratumMessageBitcoin smsg(line);
    ASSERT_EQ(smsg.getId(), "4");
    ASSERT_EQ(smsg.isStringId(), false);
  }

  {
    string line = "{\"error\": null, \"id\": \"ksadsf\", \"result\": true}";
    StratumMessageBitcoin smsg(line);
    ASSERT_EQ(smsg.getId(), "ksadsf");
    ASSERT_EQ(smsg.isStringId(), true);
  }
}
