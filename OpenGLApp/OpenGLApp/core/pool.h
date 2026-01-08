#ifndef POOL_H
#define POOL_H

#include <vector>
#include <memory>
#include <utility>
#include <functional>

//tipo generico, per poter creare pool per oggetti di tipo diverso
template <typename T>

class ObjectPool {
private:
	//vettore in cui vengono inseriti gli oggetti
	std::vector<std::shared_ptr<T>> pool;
	std::function<std::shared_ptr<T>()> factory;

public:
	template <typename... Args>
	ObjectPool(int startingSize, Args&&... args) {
		factory = [args...]() {
			return std::make_shared<T>(args...);
			};

		for (int i = 0; i < startingSize; i++) {
			pool.push_back(factory());
			//printf("aggiunto un oggetto alla pool\n");
		}
	}

	//per prendere un oggetto dalla pool
	std::shared_ptr<T> getInstance() {
		//se sono finiti gli oggetti nella pool ne creo uno nuovo e ritorno quello
		if (pool.empty()) {
			printf("istanza creata");
			return factory();
		}

		//altrimenti ne prendo uno dalla pool
		std::shared_ptr<T> instance = pool.back();
		pool.pop_back();
		//printf("sto prendendo un oggetto dalla pool\n");
		return instance;
	}

	int getSize() {
		return (int)pool.size();
	}

	void returnToPool(std::shared_ptr<T> instance) {
		pool.emplace_back(instance);
	}
};

#endif