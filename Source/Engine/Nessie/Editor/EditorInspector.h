// Inspector.h
#pragma once
#include <memory>
#include <string>
#include <unordered_map>
#include "EditorCore.h"
#include "Nessie/Core/TypeInfo.h"
#include "Nessie/Core/String/FormatString.h"
#include "Nessie/Core/Memory/StrongPtr.h"

namespace nes
{
    class WorldBase;

    //----------------------------------------------------------------------------------------------------
    /// @brief : Passed into EditorInspector draw calls, which contains the current selected objects and
    ///     the current world.
    //----------------------------------------------------------------------------------------------------
    struct InspectorContext
    {
        std::vector<uint64>     m_selectionIDs{};   // Current selections that this inspector is being used for.
        StrongPtr<WorldBase>    m_pWorld = nullptr; // The World context.
    };
    
    enum class EInspectorLevel : uint8
    {
        None = 0,               // No restriction on the Inspector. It will always be drawn if needed..
        DebugOnly = NES_BIT(0), // Only in debug builds will this show.
        Internal = NES_BIT(1),  // Internal inspectors are only for development.
    };
    NES_DEFINE_BIT_OPERATIONS_FOR_ENUM(EInspectorLevel)

    //----------------------------------------------------------------------------------------------------
    /// @brief : Base class for an EditorInspector, but you should inherit from EditorInspector<Type>. This
    ///     just acts as the untemplated interface object.
    //----------------------------------------------------------------------------------------------------
    class EditorInspectorBase
    {
    public:
        EditorInspectorBase() = default;
        EditorInspectorBase(const EInspectorLevel& flags) : m_flags(flags) {}
        virtual ~EditorInspectorBase() = default;

        //----------------------------------------------------------------------------------------------------
        /// @brief : This is the base, untyped draw function.
        /// @param pTarget : Pointer to the object being inspected.
        /// @param context : Contains the current selection(s) and World.
        //----------------------------------------------------------------------------------------------------
        virtual void                Draw(void* pTarget, const InspectorContext& context) = 0;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Defined by EditorInspector<Type>. Returns the TypeID of the object for a given Inspector.
        //----------------------------------------------------------------------------------------------------
        virtual entt::id_type       GetTargetTypeID() const = 0;

        //----------------------------------------------------------------------------------------------------
        /// @brief : Defined by EditorInspector<Type>. Returns the full name of the object for a given Inspector.
        ///     For 'EditorInspector<IDComponent>', this will return 'class nes::IDComponent'. If you want just
        ///     'IDComponent', use GetTargetShortTypename().
        //----------------------------------------------------------------------------------------------------
        virtual std::string_view    GetTargetTypename() const = 0;

        //----------------------------------------------------------------------------------------------------
        /// @brief : For 'EditorInspector<IDComponent>' this will return 'IDComponent'.    
        //----------------------------------------------------------------------------------------------------
        std::string                 GetTargetShortTypename() const;

        //----------------------------------------------------------------------------------------------------
        /// @brief : If an EditorInspector is internal, it will only be shown if the Internal flag is set and
        ///  only in debug builds. An internal inspector is only for debugging purposes, and isn't really for
        ///  editing.
        //----------------------------------------------------------------------------------------------------
        bool                        IsInternal() const     { return m_flags & EInspectorLevel::Internal; }

        //----------------------------------------------------------------------------------------------------
        /// @brief : If an EditorInspector is debug only, it will only be shown if the DebugOnly flag is set and
        ///  only in debug builds. A debug only inspector is only for debugging purposes, but is editable when
        ///  needed.
        //----------------------------------------------------------------------------------------------------
        bool                        IsDebugOnly() const    { return m_flags & EInspectorLevel::DebugOnly; }
    
    protected:
        EInspectorLevel             m_flags = EInspectorLevel::None; // The level that this Inspector should be shown in.
    };

    //----------------------------------------------------------------------------------------------------
    /// @brief : Base class for all Editor Inspectors. The template parameter is the Type that will be showing.
    ///     You will need to override DrawImpl() in derived classes.
    /// @note : There is a single inspector instance per type. The same EditorInspector<IDComponent> will be used
    ///     to draw all IDComponents.
    ///	@tparam Type : The type that this inspector will be inspecting. I.e., EditorInspector<IDComponent> will
    /// be used to show IDComponent information.
    //----------------------------------------------------------------------------------------------------
    template <typename Type>
    class EditorInspector : public EditorInspectorBase
    {
    public:
        using TargetType = Type;
        
    public:
        EditorInspector() = default;
        EditorInspector(const EInspectorLevel& flags) : EditorInspectorBase(flags) {}

        //----------------------------------------------------------------------------------------------------
        /// @brief : This casts the un-typed target to the TargetType and calls DrawImpl().
        //----------------------------------------------------------------------------------------------------
        virtual void                Draw(void* pTarget, const InspectorContext& context) override final;
        virtual entt::id_type       GetTargetTypeID() const override final      { return entt::type_id<TargetType>().hash(); }
        virtual std::string_view    GetTargetTypename() const override final    { return entt::type_name<TargetType>(); }
    
    private:
        //----------------------------------------------------------------------------------------------------
        /// @brief : This should be overriden by derived classes to perform the draw logic of the inspector.
        ///	@param pTarget : Pointer to the object that is being inspected.
        ///	@param context : The context contains the World that we are in, and the current selections in the editor.
        //----------------------------------------------------------------------------------------------------
        virtual void                DrawImpl([[maybe_unused]] TargetType* pTarget, [[maybe_unused]] const InspectorContext& context) {}
    };

    //----------------------------------------------------------------------------------------------------
    /// @brief : Checks that the Type inherits from EditorInspector.
    //----------------------------------------------------------------------------------------------------
    template <typename Type>
    concept IsValidEntityInspector = std::is_base_of_v<EditorInspectorBase, Type>
        && requires() { typename Type::TargetType; }
        && std::is_base_of_v<EditorInspector<typename Type::TargetType>, Type>;

    //----------------------------------------------------------------------------------------------------
    /// @brief : The Editor Inspector Registry maps a TargetTypes to their register Inspector classes.
    ///     There is a single inspector instance per type. The same EditorInspector<IDComponent> will be used
    ///     to draw all IDComponents.
    //----------------------------------------------------------------------------------------------------
    class EditorInspectorRegistry
    {
    public:
        //----------------------------------------------------------------------------------------------------
        /// @brief : Register an Inspector class to the registry. A valid Editor Inspector inherits from
        ///  EditorInspector<Type>, where the Type is the object type being inspected. An inspector must be
        ///  registered to be used in the Inspector Window.
        //----------------------------------------------------------------------------------------------------
        template <IsValidEntityInspector InspectorType>
        static void                                 RegisterInspector();

        //----------------------------------------------------------------------------------------------------
        /// @brief : Returns if an Inspector has been registered for the given type. For example, if an
        /// EditorInspector<IDComponent> has been registered, then HasInspector<IDComponent> returns true.
        //----------------------------------------------------------------------------------------------------
        template <typename Type>
        static bool                                 HasInspector();

        //----------------------------------------------------------------------------------------------------
        /// @brief : Returns if an EditorInspector has been registered for the given type ID. For example, if an
        /// EditorInspector<IDComponent> has been registered, then HasInspector(entt::type_id<IDComponent>().hash()) returns true.
        //----------------------------------------------------------------------------------------------------
        static bool                                 HasInspector(const entt::id_type typeID);

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the EditorInspector instance for a given type. The inspector class must first be
        ///  registered to be used. Returns nullptr if not registered.
        //----------------------------------------------------------------------------------------------------
        template <typename Type>
        static std::shared_ptr<EditorInspector<Type>> GetInspector();

        //----------------------------------------------------------------------------------------------------
        /// @brief : Get the EditorInspector instance for a given type ID. The inspector class must first be
        ///  registered to be used. Returns nullptr if not registered.
        //----------------------------------------------------------------------------------------------------
        static std::shared_ptr<EditorInspectorBase> GetInspector(const entt::id_type typeID);
    
    private:
        static inline std::unordered_map<uint32, std::shared_ptr<EditorInspectorBase>> s_inspectors{};
    };

    template <typename Type>
    void EditorInspector<Type>::Draw(void* pTarget, const InspectorContext& context)
    {
        if (!pTarget)
            return;
            
        TargetType* pCastTarget = static_cast<TargetType*>(pTarget);
        DrawImpl(pCastTarget, context);
    }

    template <IsValidEntityInspector InspectorType>
    void EditorInspectorRegistry::RegisterInspector()
    {
        // Generate a TypeID for the target type.
        const entt::type_info& typeInfo = entt::type_id<typename InspectorType::TargetType>();
        const uint32 typeID = typeInfo.hash();
            
        if (s_inspectors.contains(typeID))
        {
            NES_ERROR("Failed to registry Inspector for type '{}'! Inspector already registered for the type!", typeInfo.name());
        }

        s_inspectors.emplace(typeID, std::make_shared<InspectorType>());
    }

    template <typename Type>
    bool EditorInspectorRegistry::HasInspector()
    {
        const entt::type_info& typeInfo = entt::type_id<Type>();
        return HasInspector(typeInfo.hash());
    }

    template <typename Type>
    std::shared_ptr<EditorInspector<Type>> EditorInspectorRegistry::GetInspector()
    {
        const entt::type_info& typeInfo = entt::type_id<Type>();
        const uint32 typeID = typeInfo.hash();

        if (HasInspector(typeID))
        {
            return std::dynamic_pointer_cast<EditorInspector<Type>>(s_inspectors.at(typeID));    
        }
            
        return nullptr;
    }
}
