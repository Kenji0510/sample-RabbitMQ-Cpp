#include <iostream>
#include <string>
#include <memory>
#include <open3d_cuda/include/open3d/utility/Eigen.h>
#include <open3d/Open3D.h>
#include <dotenv.h>
#include <SimpleAmqpClient/SimpleAmqpClient.h>
#include <json/json.h>
#include <vector>

void voxelization(
    std::shared_ptr<open3d::geometry::PointCloud> &sample_pcd,
    std::shared_ptr<open3d::geometry::PointCloud> &sample_pcd_d)
{
    // Voxelization
    double voxel_size = 0.1;
    sample_pcd_d = sample_pcd->VoxelDownSample(voxel_size);
    std::cout << "Downsampled point cloud has " << sample_pcd_d->points_.size() << " points." << std::endl;
    return;
}

int main(int argc, char *argv[])
{
    try
    {
        auto sample_pcd = std::make_shared<open3d::geometry::PointCloud>();
        auto sample_pcd_d = std::make_shared<open3d::geometry::PointCloud>();
        if (open3d::io::ReadPointCloud("../data/pcd/lab-room.pcd", *sample_pcd) == false)
        {
            std::cerr << "Failed to read pcd file" << std::endl;
            return 1;
        }
        voxelization(sample_pcd, sample_pcd_d);

        auto pcd_data = std::make_shared<std::vector<double>>();
        for (auto &point : sample_pcd_d->points_)
        {
            pcd_data->push_back(point(0)); // x
            pcd_data->push_back(point(1)); // y
            pcd_data->push_back(point(2)); // z
        }
        std::cout << "Point cloud data: " << (*pcd_data)[0] << ", " << (*pcd_data)[1] << ", " << (*pcd_data)[2] << std::endl;

        Json::Value data;
        data["num_points"] = static_cast<Json::UInt64>(sample_pcd_d->points_.size());
        data["color"] = "#FF0000";

        Json::Value points(Json::arrayValue);
        for (const auto &value : *pcd_data)
        {
            points.append(value);
        }
        data["points"] = points;

        Json::StreamWriterBuilder writer;
        std::string json_data = Json::writeString(writer, data);

        // auto connection = AmqpClient::Channel::Create("amqp://guest:guest@192.168.0.4:5672");
        auto connection = AmqpClient::Channel::Create("192.168.0.4", 5672, "guest", "guest", "/");

        const std::string queue_name = "hello";
        // const std::string queue_name = "matching-pcd";
        // connection->DeclareQueue(queue_name, false, false, false, false);

        const std::string exchange_name = "amq.direct";
        // connection->DeclareExchange(exchange_name, AmqpClient::Channel::EXCHANGE_TYPE_DIRECT);

        for (int i = 0; i < 100; i++)
        {
            connection->BasicPublish(exchange_name, queue_name, AmqpClient::BasicMessage::Create(json_data));
            std::cout << "Sent message count: " << i << std::endl;
            // std::this_thread::sleep_for(std::chrono::seconds(1));
            std::this_thread::sleep_for(std::chrono::milliseconds(300));
        }

        std::cout << "Sent message: " << std::endl;

        connection.reset();
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}