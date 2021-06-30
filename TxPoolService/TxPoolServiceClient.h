#pragma once

#include "TransactionSubmitResult.h"
#include "TxPoolService.h"
#include "bcos-framework/interfaces/txpool/TxPoolInterface.h"
#include "../protocols/TransactionSubmitResultImpl.h"
#include "../protocols/TransactionImpl.h"
#include "../Common/ErrorConverter.h"
#include <memory>

namespace bcostars {

class TxPoolServiceClient : public bcos::txpool::TxPoolInterface {
public:
  void asyncSubmit(bcos::bytesPointer _tx, bcos::protocol::TxSubmitCallback _txSubmitCallback, std::function<void(bcos::Error::Ptr)> _onRecv) override {
    class Callback : public bcostars::TxPoolServicePrxCallback {
    public:
      Callback(bcos::protocol::TxSubmitCallback callback, bcos::crypto::CryptoSuite::Ptr cryptoSuite) : m_callback(callback), m_cryptoSuite(cryptoSuite) {}

      void callback_asyncSubmit(const bcostars::Error &ret, const bcostars::TransactionSubmitResult &result) override {
        auto bcosResult = std::make_shared<bcostars::protocol::TransactionSubmitResultImpl>(m_cryptoSuite);
        bcosResult->setInner(result);
        m_callback(toBcosError(ret), bcosResult);
      }
      void callback_asyncSubmit_exception(tars::Int32 ret) override { m_callback(toBcosError(ret), nullptr); }

    private:
      bcos::protocol::TxSubmitCallback m_callback;
      bcos::crypto::CryptoSuite::Ptr m_cryptoSuite;
    };

    m_proxy->async_asyncSubmit(new Callback(_txSubmitCallback, m_cryptoSuite), *_tx);
  }

  void asyncSealTxs(size_t _txsLimit, bcos::txpool::TxsHashSetPtr _avoidTxs,
                    std::function<void(bcos::Error::Ptr, bcos::crypto::HashListPtr, bcos::crypto::HashListPtr)> _sealCallback) override {
    class Callback : public bcostars::TxPoolServicePrxCallback {
    public:
      Callback(std::function<void(bcos::Error::Ptr, bcos::crypto::HashListPtr, bcos::crypto::HashListPtr)> callback) : m_callback(callback) {}

      void callback_asyncSealTxs(const bcostars::Error &ret, const vector<vector<tars::UInt8>> &return1, const vector<vector<tars::UInt8>> &return2) override {
        auto list1 = std::make_shared<bcos::crypto::HashList>();
        for (auto &it : return1) {
          list1->push_back(bcos::crypto::HashType(it));
        }

        auto list2 = std::make_shared<bcos::crypto::HashList>();
        for (auto &it : return2) {
          list2->push_back(bcos::crypto::HashType(it));
        }

        m_callback(toBcosError(ret), list1, list2);
      }

      void callback_asyncSealTxs_exception(tars::Int32 ret) override { m_callback(toBcosError(ret), nullptr, nullptr); }

    private:
      std::function<void(bcos::Error::Ptr, bcos::crypto::HashListPtr, bcos::crypto::HashListPtr)> m_callback;
    };

    vector<vector<tars::UInt8>> tarsAvoidTxs;
    for (auto &it : *_avoidTxs) {
      tarsAvoidTxs.push_back(it.asBytes());
    }

    m_proxy->async_asyncSealTxs(new Callback(_sealCallback), _txsLimit, tarsAvoidTxs);
  }

  void asyncMarkTxs(bcos::crypto::HashListPtr _txsHash, bool _sealedFlag, std::function<void(bcos::Error::Ptr)> _onRecvResponse) override {
    class Callback : public bcostars::TxPoolServicePrxCallback {
    public:
      Callback(std::function<void(bcos::Error::Ptr)> callback) : m_callback(callback) {}

      void callback_asyncMarkTxs(const bcostars::Error &ret) override { m_callback(toBcosError(ret)); }

      void callback_asyncMarkTxs_exception(tars::Int32 ret) override { m_callback(toBcosError(ret)); }

    private:
      std::function<void(bcos::Error::Ptr)> m_callback;
    };

    vector<vector<tars::UInt8>> txHashList;
    for (auto &it : *_txsHash) {
      txHashList.push_back(it.asBytes());
    }

    m_proxy->async_asyncMarkTxs(new Callback(_onRecvResponse), txHashList, _sealedFlag);
  }

  void asyncVerifyBlock(bcos::crypto::PublicPtr _generatedNodeID, bcos::bytesConstRef const &_block,
                        std::function<void(bcos::Error::Ptr, bool)> _onVerifyFinished) override {
    class Callback : public bcostars::TxPoolServicePrxCallback {
    public:
      Callback(std::function<void(bcos::Error::Ptr, bool)> callback) : m_callback(callback) {}

      void callback_asyncVerifyBlock(const bcostars::Error &ret, tars::Bool result) override { m_callback(toBcosError(ret), result); }

      void callback_asyncVerifyBlock_exception(tars::Int32 ret) override { m_callback(toBcosError(ret), false); }

    private:
      std::function<void(bcos::Error::Ptr, bool)> m_callback;
    };

    m_proxy->async_asyncVerifyBlock(new Callback(_onVerifyFinished), _generatedNodeID->data(), _block.toBytes());
  }

  void asyncFillBlock(bcos::crypto::HashListPtr _txsHash, std::function<void(bcos::Error::Ptr, bcos::protocol::TransactionsPtr)> _onBlockFilled) override {
    class Callback : public bcostars::TxPoolServicePrxCallback {
    public:
      Callback(std::function<void(bcos::Error::Ptr, bcos::protocol::TransactionsPtr)> callback, bcos::crypto::CryptoSuite::Ptr cryptoSuite)
          : m_callback(callback), m_cryptoSuite(cryptoSuite) {}

      void callback_asyncFillBlock(const bcostars::Error &ret, const vector<bcostars::Transaction> &filled) override {
        auto txs = std::make_shared<bcos::protocol::Transactions>();
        for (auto &it : filled) {
          auto tx = std::make_shared<bcostars::protocol::TransactionImpl>(m_cryptoSuite);
          tx->setInner(it);
          txs->push_back(tx);
        }
        m_callback(toBcosError(ret), txs);
      }

      void callback_asyncFillBlock_exception(tars::Int32 ret) override { m_callback(toBcosError(ret), nullptr); }

    private:
      std::function<void(bcos::Error::Ptr, bcos::protocol::TransactionsPtr)> m_callback;
      bcos::crypto::CryptoSuite::Ptr m_cryptoSuite;
    };

    vector<vector<tars::UInt8>> hashList;
    for (auto hashData : *_txsHash) {
      hashList.emplace_back(hashData.asBytes());
    }

    m_proxy->async_asyncFillBlock(new Callback(_onBlockFilled, m_cryptoSuite), hashList);
  }

  void asyncNotifyBlockResult(bcos::protocol::BlockNumber _blockNumber, bcos::protocol::TransactionSubmitResultsPtr _txsResult,
                              std::function<void(bcos::Error::Ptr)> _onNotifyFinished) override {
    class Callback : public bcostars::TxPoolServicePrxCallback {
    public:
      Callback(std::function<void(bcos::Error::Ptr)> callback) : m_callback(callback) {}

      void callback_asyncNotifyBlockResult(const bcostars::Error &ret) override { m_callback(toBcosError(ret)); }

      void callback_asyncNotifyBlockResult_exception(tars::Int32 ret) override { m_callback(toBcosError(ret)); }

    private:
      std::function<void(bcos::Error::Ptr)> m_callback;
    };

    vector<bcostars::TransactionSubmitResult> resultList;
    for (auto &it : *_txsResult) {
      resultList.emplace_back(std::dynamic_pointer_cast<bcostars::protocol::TransactionSubmitResultImpl>(it)->inner());
    }

    m_proxy->async_asyncNotifyBlockResult(new Callback(_onNotifyFinished), _blockNumber, resultList);
  }

  void asyncNotifyTxsSyncMessage(bcos::Error::Ptr _error, bcos::crypto::NodeIDPtr _nodeID, bcos::bytesConstRef _data,
                                 std::function<void(bcos::bytesConstRef _respData)> _sendResponse,
                                 std::function<void(bcos::Error::Ptr _error)> _onRecv) override {
    class Callback : public bcostars::TxPoolServicePrxCallback {
    public:
      Callback(std::function<void(bcos::bytesConstRef _respData)> callback) : m_callback(callback) {}

      void callback_asyncNotifyTxsSyncMessage(const bcostars::Error& ret,  const vector<tars::UInt8>& respData) override {}

      void callback_asyncNotifyTxsSyncMessage_exception(tars::Int32 ret) override {}

    private:
      std::function<void(bcos::bytesConstRef _respData)> m_callback;
    };
  }

  void notifyConnectedNodes(bcos::crypto::NodeIDSet const &_connectedNodes, std::function<void(bcos::Error::Ptr)> _onRecvResponse) override {}
  void notifyConsensusNodeList(bcos::consensus::ConsensusNodeList const &_consensusNodeList, std::function<void(bcos::Error::Ptr)> _onRecvResponse) override {}
  void notifyObserverNodeList(bcos::consensus::ConsensusNodeList const &_observerNodeList, std::function<void(bcos::Error::Ptr)> _onRecvResponse) override {}

private:
  bcostars::TxPoolServicePrx m_proxy;
  bcos::crypto::CryptoSuite::Ptr m_cryptoSuite;
};

} // namespace bcostars