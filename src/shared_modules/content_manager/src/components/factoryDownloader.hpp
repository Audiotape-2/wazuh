/*
 * Wazuh content manager
 * Copyright (C) 2015, Wazuh Inc.
 * April 14, 2023.
 *
 * This program is free software; you can redistribute it
 * and/or modify it under the terms of the GNU General Public
 * License (version 2) as published by the FSF - Free Software
 * Foundation.
 */

#ifndef _FACTORY_DOWNLOADER_HPP
#define _FACTORY_DOWNLOADER_HPP

#include "APIDownloader.hpp"
#include "CtiApiDownloader.hpp"
#include "HTTPRequest.hpp"
#include "S3Downloader.hpp"
#include "json.hpp"
#include "offlineDownloader.hpp"
#include "updaterContext.hpp"
#include "utils/chainOfResponsability.hpp"
#include <iostream>
#include <memory>
#include <string>

/**
 * @class FactoryDownloader
 *
 * @brief Class in charge of creating the content downloader.
 *
 */
class FactoryDownloader final
{
public:
    /**
     * @brief Create the content downloader based on the contentSource value.
     *
     * @param config Configurations.
     * @return std::shared_ptr<AbstractHandler<std::shared_ptr<UpdaterContext>>>
     */
    static std::shared_ptr<AbstractHandler<std::shared_ptr<UpdaterContext>>> create(const nlohmann::json& config)
    {
        auto const downloaderType {config.at("contentSource").get<std::string>()};
        std::cout << "Creating '" << downloaderType << "' downloader" << std::endl;

        if ("api" == downloaderType)
        {
            return std::make_shared<APIDownloader>(HTTPRequest::instance());
        }
        if ("cti-api" == downloaderType)
        {
            return std::make_shared<CtiApiDownloader>(HTTPRequest::instance());
        }
        if ("s3" == downloaderType)
        {
            return std::make_shared<S3Downloader>();
        }
        if ("offline" == downloaderType)
        {
            return std::make_shared<OfflineDownloader>();
        }

        throw std::invalid_argument {"Invalid 'contentSource' type: " + downloaderType};
    }
};

#endif // _FACTORY_DOWNLOADER_HPP